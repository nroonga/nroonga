#include "nroonga.h"

namespace nroonga {

Nan::Persistent<v8::Function> groonga_context_constructor;

v8::Local<v8::String> Database::optionsToCommandString(
    const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Bad parameter");
    return Nan::New("").ToLocalChecked();
  }
  if (info.Length() == 1) {
    return info[0]->ToString();
  }
  if (info.Length() >= 2 && !info[1]->IsObject()) {
    return info[0]->ToString();
  }

  v8::Local<v8::Object> options = info[1]->ToObject();
  v8::Local<v8::Array> props =
    Nan::GetOwnPropertyNames(options).ToLocalChecked();

  v8::Local<v8::String> commandString = info[0]->ToString();
  Nan::JSON NanJSON;
  for (int i = 0, l = props->Length(); i < l; i++) {
    v8::Local<v8::Value> key = props->Get(i);
    v8::Local<v8::Value> value = Nan::Get(options, key).ToLocalChecked();
    if (value->IsNull()) {
      continue;
    }

    commandString = v8::String::Concat(commandString,
                                       Nan::New(" --").ToLocalChecked());
    commandString = v8::String::Concat(commandString, key->ToString());
    commandString = v8::String::Concat(commandString,
                                       Nan::New(" ").ToLocalChecked());
    commandString = v8::String::Concat(
        commandString,
        NanJSON.Stringify(value->ToObject()).ToLocalChecked()->ToString());
  }
  return commandString;
}

void Database::Initialize(v8::Local<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);

  tpl->SetClassName(Nan::New("Database").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "command", Database::Command);
  Nan::SetPrototypeMethod(tpl, "commandSync", Database::CommandSync);
  Nan::SetPrototypeMethod(tpl, "close", Database::Close);

  groonga_context_constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("Database").ToLocalChecked(),
               tpl->GetFunction());
}

void Database::New(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if (!info.IsConstructCall()) {
    Nan::ThrowTypeError("Use the new operator to create new Database objects");
    return;
  }

  Database *db = new Database();
  db->closed = true;
  grn_ctx *ctx = &db->context;
  grn_ctx_init(ctx, 0);
  if (info[0]->IsUndefined()) {
    db->database = grn_db_create(ctx, NULL, NULL);
  } else if (info[0]->IsString()) {
    v8::String::Utf8Value path(info[0]->ToString());
    if (info.Length() > 1 && info[1]->IsTrue()) {
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
  db->Wrap(info.Holder());
  info.GetReturnValue().Set(info.This());
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

void Database::Close(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Database *db = ObjectWrap::Unwrap<Database>(info.Holder());

  if (db->closed) {
    Nan::ThrowTypeError("Database already closed");
    return;
  }

  if (db->Cleanup()) {
    info.GetReturnValue().Set(Nan::True());
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
  Nan::HandleScope scope;

  Baton* baton = static_cast<Baton*>(req->data);
  const unsigned argc = 2;
  v8::Handle<v8::Value> argv[argc];
  if (baton->error) {
    argv[0] = v8::Exception::Error(
        Nan::New(baton->context.errbuf).ToLocalChecked());
    argv[1] = Nan::Undefined();
  } else {
    argv[0] = Nan::Null();

    v8::Local<v8::String> string = Nan::New(
        baton->result, baton->result_length).ToLocalChecked();
    Nan::TryCatch tc;
    Nan::JSON NanJSON;
    Nan::MaybeLocal<v8::Value> parse_value = NanJSON.Parse(string);
    if (tc.HasCaught() || parse_value.IsEmpty()) {
      argv[1] = string;
    } else {
      argv[1] = parse_value.ToLocalChecked();
    }
  }
  Nan::MakeCallback(Nan::GetCurrentContext()->Global(),
                    Nan::New<v8::Function>(baton->callback),
                    argc, argv);
  grn_ctx_fin(&baton->context);
  delete baton;
}

void Database::Command(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Database *db = ObjectWrap::Unwrap<Database>(info.Holder());
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Bad parameter");
    return;
  }

  v8::Local<v8::Function> callback;
  if (info.Length() == 2) {
    if (!info[1]->IsFunction()) {
      Nan::ThrowTypeError("Second argument must be a callback function");
      return;
    }
    callback = info[1].As<v8::Function>();
  } else if (info.Length() == 3) {
    if (info[1]->IsFunction()) {
      callback = info[1].As<v8::Function>();
    } else if (info[2]->IsFunction()) {
      callback = info[2].As<v8::Function>();
    } else {
      Nan::ThrowTypeError("Second or Third argument must be a callback function");
      return;
    }
  } else {
    Nan::ThrowTypeError("Bad parameter");
    return;
  }

  if (db->closed) {
    Nan::ThrowTypeError("Database already closed");
    return;
  }

  Baton* baton = new Baton();
  baton->request.data = baton;
  baton->callback.Reset(callback);

  v8::String::Utf8Value command(optionsToCommandString(info));
  baton->database = db->database;

  baton->command = std::string(*command, command.length());
  uv_queue_work(uv_default_loop(),
                &baton->request,
                CommandWork,
                (uv_after_work_cb)CommandAfter);

  info.GetReturnValue().Set(Nan::Undefined());
}

void Database::CommandSync(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Database *db = ObjectWrap::Unwrap<Database>(info.Holder());
  if (info.Length() < 1 || !info[0]->IsString()) {
    Nan::ThrowTypeError("Bad parameter");
    return;
  }

  int rc = -1;
  grn_ctx *ctx = &db->context;
  char *result;
  unsigned int result_length;
  int flags;
  v8::String::Utf8Value command(optionsToCommandString(info));

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

  v8::Local<v8::String> string = Nan::New(result,
                                          result_length).ToLocalChecked();
  Nan::TryCatch tc;
  Nan::JSON NanJSON;
  Nan::MaybeLocal<v8::Value> parse_value = NanJSON.Parse(string);
  if (tc.HasCaught() || parse_value.IsEmpty()) {
    info.GetReturnValue().Set(string);
    return;
  }
  info.GetReturnValue().Set(parse_value.ToLocalChecked());
}

void InitNroonga(v8::Local<v8::Object> exports) {
  grn_init();
  Database::Initialize(exports);
}

} // namespace nroonga

NODE_MODULE(nroonga_bindings, nroonga::InitNroonga);
