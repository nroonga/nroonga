#include <node_buffer.h>
#include <nan.h>
#include "nroonga.h"

namespace nroonga {

using namespace v8;

Nan::Persistent<Function> groonga_context_constructor;

void Database::Initialize(Handle<Object> exports) {
  Isolate* isolate = Isolate::GetCurrent();

  Local<FunctionTemplate> t = FunctionTemplate::New(isolate, New);

  t->SetClassName(String::NewFromUtf8(isolate, "Database"));
  t->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(t, "commandString", Database::CommandString);
  NODE_SET_PROTOTYPE_METHOD(t, "commandSyncString", Database::CommandSyncString);
  NODE_SET_PROTOTYPE_METHOD(t, "close", Database::Close);

  groonga_context_constructor.Reset(t->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Database"), t->GetFunction());
}

void Database::New(const FunctionCallbackInfo<Value>& args) {
  if (!args.IsConstructCall()) {
    Nan::ThrowTypeError("Use the new operator to create new Database objects");
    return;
  }

  Database *db = new Database();
  db->closed = true;
  grn_ctx *ctx = &db->context;
  grn_ctx_init(ctx, 0);
  if (args[0]->IsUndefined()) {
    db->database = grn_db_create(ctx, NULL, NULL);
  } else if (args[0]->IsString()) {
    String::Utf8Value path(args[0]->ToString());
    if (args.Length() > 1 && args[1]->IsTrue()) {
      db->database = grn_db_open(ctx, *path);
    } else {
      GRN_DB_OPEN_OR_CREATE(ctx, *path, NULL, db->database);
    }
    if (ctx->rc != GRN_SUCCESS) {
      Nan::ThrowTypeError(ctx->errbuf);
      return;
    }
  } else {
    Nan::ThrowTypeError("Bad parameter");
    return;
  }

  db->closed = false;
  db->Wrap(args.Holder());
  args.GetReturnValue().Set(args.This());
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

void Database::Close(const FunctionCallbackInfo<Value>& args) {
  Database *db = ObjectWrap::Unwrap<Database>(args.Holder());

  if (db->closed) {
    Nan::ThrowTypeError("Database already closed");
    return;
  }

  if (db->Cleanup()) {
    args.GetReturnValue().Set(Nan::True());
    return;
  }
  Nan::ThrowTypeError("Failed to close the database");
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
}

void Database::CommandAfter(uv_work_t* req) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  Baton* baton = static_cast<Baton*>(req->data);
  Handle<Value> argv[2];
  if (baton->error) {
    argv[0] = Exception::Error(Nan::New(baton->context.errbuf).ToLocalChecked());
    argv[1] = Nan::Null();
  } else {
    argv[0] = Nan::Null();
    argv[1] = Nan::NewBuffer(baton->result, baton->result_length)
        .ToLocalChecked();
  }
  Local<Function>::New(isolate, baton->callback)
      ->Call(Nan::GetCurrentContext()->Global(), 2, argv);
  grn_ctx_fin(&baton->context);
  delete baton;
}

void Database::CommandString(const FunctionCallbackInfo<Value>& args) {
  Database *db = ObjectWrap::Unwrap<Database>(args.Holder());
  if (args.Length() < 1 || !args[0]->IsString()) {
    Nan::ThrowTypeError("Bad parameter");
    return;
  }

  Local<Function> callback;
  if (args.Length() >= 2) {
    if (!args[1]->IsFunction()) {
      Nan::ThrowTypeError("Second argument must be a callback function");
      return;
    }
    callback = Local<Function>::Cast(args[1]);
  }

  if (db->closed) {
    Nan::ThrowTypeError("Database already closed");
    return;
  }

  Baton* baton = new Baton();
  baton->request.data = baton;
  baton->callback.Reset(callback);

  String::Utf8Value command(args[0]->ToString());
  baton->database = db->database;

  baton->command = std::string(*command, command.length());
  uv_queue_work(uv_default_loop(),
      &baton->request,
      CommandWork,
      (uv_after_work_cb)CommandAfter
      );

  args.GetReturnValue().Set(Nan::Undefined());
}

void Database::CommandSyncString(const FunctionCallbackInfo<Value>& args) {
  Database *db = ObjectWrap::Unwrap<Database>(args.Holder());
  if (args.Length() < 1 || !args[0]->IsString()) {
    Nan::ThrowTypeError("Bad parameter");
    return;
  }

  int rc = -1;
  grn_ctx *ctx = &db->context;
  char *result;
  unsigned int result_length;
  int flags;
  String::Utf8Value command(args[0]->ToString());

  if (db->closed) {
    Nan::ThrowTypeError("Database already closed");
    return;
  }

  rc = grn_ctx_send(ctx, *command, command.length(), 0);
  if (rc < 0) {
    Nan::ThrowTypeError("grn_ctx_send returned error");
    return;
  }
  if (ctx->rc != GRN_SUCCESS) {
    Nan::ThrowTypeError(ctx->errbuf);
    return;
  }
  grn_ctx_recv(ctx, &result, &result_length, &flags);
  if (ctx->rc < 0) {
    Nan::ThrowTypeError("grn_ctx_recv returned error");
    return;
  }

  args.GetReturnValue().Set(
      Nan::NewBuffer(result, result_length).ToLocalChecked()->ToString());
}

void InitNroonga(Handle<Object> target) {
  grn_init();
  Database::Initialize(target);
}

} // namespace nroonga

NODE_MODULE(nroonga_bindings, nroonga::InitNroonga);
