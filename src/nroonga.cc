#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <groonga.h>
#include "nroonga.h"

namespace nroonga {

using namespace v8;

static Persistent<FunctionTemplate> groonga_context_constructor;

void Database::Initialize(Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(Database::New);
  groonga_context_constructor = Persistent<FunctionTemplate>::New(t);

  t->InstanceTemplate()->SetInternalFieldCount(1);
  t->SetClassName(String::NewSymbol("Database"));

  NODE_SET_PROTOTYPE_METHOD(t, "commandString", Database::CommandString);
  NODE_SET_PROTOTYPE_METHOD(t, "commandSyncString", Database::CommandSyncString);
  NODE_SET_PROTOTYPE_METHOD(t, "close", Database::Close);

  target->Set(String::NewSymbol("Database"), t->GetFunction());
}

Handle<Value> Database::New(const Arguments& args) {
  HandleScope scope;

  if (!args.IsConstructCall()) {
    return ThrowException(Exception::TypeError(
          String::New("Use the new operator to create new Database objects"))
        );
  }

  Database *db = new Database();
  db->closed = true;
  grn_ctx *ctx = &db->context;
  grn_ctx_init(ctx, 0);
  if (args[0]->IsUndefined()) {
    db->database = grn_db_create(ctx, NULL, NULL);
  } else if(args[0]->IsString()) {
    String::Utf8Value path(args[0]->ToString());
    if (args.Length() > 1 && args[1]->IsTrue()) {
      db->database = grn_db_open(ctx, *path);
    } else {
      GRN_DB_OPEN_OR_CREATE(ctx, *path, NULL, db->database);
    }
    if (ctx->rc != GRN_SUCCESS) {
      return ThrowException(Exception::Error(String::New(ctx->errbuf)));
    }
  } else {
    return ThrowException(Exception::TypeError(String::New("Bad parameter")));
  }

  db->closed = false;
  db->Wrap(args.Holder());
  return args.This();
}

bool Database::Cleanup() {
  if (grn_obj_close(&context, database) != GRN_SUCCESS) {
    return false;
  }
  if (grn_ctx_fin(&context) != GRN_SUCCESS) {
    return false;
  }
  this->closed = true;

  return true;
}

Handle<Value> Database::Close(const Arguments& args) {
  HandleScope scope;
  Database *db = ObjectWrap::Unwrap<Database>(args.Holder());

  if (db->closed) {
    return ThrowException(Exception::Error(String::New("Database already closed")));
  }

  if (db->Cleanup()) {
    return True();
  } else {
    return ThrowException(Exception::Error(String::New("Failed to close the database")));
  }
}

void Database::CommandWork(uv_work_t* req) {
  Baton *baton = static_cast<Baton*>(req->data);
  int rc = -1;
  int flags;
  grn_ctx *ctx = &baton->context;

  grn_ctx_init(ctx, 0);
  grn_ctx_use(ctx, baton->database);
  rc = grn_ctx_send(ctx, baton->command.c_str(), baton->command.size(), 0);
  if (rc < 0) {
    baton->error = 1;
    return;
  }
  if (ctx->rc != GRN_SUCCESS) {
    baton->error = 2;
    return;
  }

  grn_ctx_recv(ctx, &baton->result, &baton->result_length, &flags);
  if (ctx->rc < 0) {
    baton->error = 3;
    return;
  }
  baton->error = 0;
  return;
}

void Database::CommandAfter(uv_work_t* req) {
  HandleScope scope;
  Baton* baton = static_cast<Baton*>(req->data);
  Handle<Value> argv[2];
  if (baton->error) {
    argv[0] = Exception::Error(String::New(baton->context.errbuf));
    argv[1] = Null();
  } else {
    argv[0] = Null();
    argv[1] = Buffer::New(baton->result, baton->result_length)->handle_;
  }
  TryCatch try_catch;
  baton->callback->Call(v8::Context::GetCurrent()->Global(), 2, argv);
  if (try_catch.HasCaught()) {
    node::FatalException(try_catch);
  }

  grn_ctx_fin(&baton->context);
  baton->callback.Dispose();
  delete baton;
}

Handle<Value> Database::CommandString(const Arguments& args) {
  HandleScope scope;
  Database *db = ObjectWrap::Unwrap<Database>(args.Holder());
  if (args.Length() < 1 || !args[0]->IsString()) {
    return ThrowException(Exception::TypeError(String::New("Bad parameter")));
  }

  Local<Function> callback;
  if (args.Length() >= 2) {
    if (!args[1]->IsFunction()) {
      return ThrowException(Exception::TypeError(String::New("Second argument must be a callback function")));
    } else {
      callback = Local<Function>::Cast(args[1]);
    }
  }

  if (db->closed) {
    return ThrowException(Exception::Error(String::New("Database already closed")));
  }

  Baton* baton = new Baton();
  baton->request.data = baton;
  baton->callback = Persistent<Function>::New(callback);

  String::Utf8Value command(args[0]->ToString());
  baton->database = db->database;

  baton->command = std::string(*command, command.length());
  uv_queue_work(uv_default_loop(),
      &baton->request,
      CommandWork,
      (uv_after_work_cb)CommandAfter
      );
  return Undefined();
}

Handle<Value> Database::CommandSyncString(const Arguments& args) {
  HandleScope scope;

  Database *db = ObjectWrap::Unwrap<Database>(args.Holder());
  if (args.Length() < 1 || !args[0]->IsString()) {
    return ThrowException(Exception::TypeError(String::New("Bad parameter")));
  }

  int rc = -1;
  grn_ctx *ctx = &db->context;
  char *result;
  unsigned int result_length;
  int flags;
  String::Utf8Value command(args[0]->ToString());

  if (db->closed) {
    return ThrowException(Exception::Error(String::New("Database already closed")));
  }

  rc = grn_ctx_send(ctx, *command, command.length(), 0);
  if (rc < 0) {
    return ThrowException(Exception::Error(String::New("grn_ctx_send returned error")));
  }
  if (ctx->rc != GRN_SUCCESS) {
    return ThrowException(Exception::Error(String::New(ctx->errbuf)));
  }
  grn_ctx_recv(ctx, &result, &result_length, &flags);
  if (ctx->rc < 0) {
    return ThrowException(Exception::Error(String::New("grn_ctx_recv returned error")));
  }

  return scope.Close(Buffer::New(result, result_length)->handle_);
}

void InitNroonga(Handle<Object> target) {
  HandleScope scope;
  grn_init();

  Database::Initialize(target);
}

} // namespace nroonga

NODE_MODULE(nroonga_bindings, nroonga::InitNroonga);
