#ifndef NROONGA_GROONGA_H
#define NROONGA_GROONGA_H
#include <nan.h>
#include <groonga.h>

namespace nroonga {

class Database : public Nan::ObjectWrap {
  grn_ctx context;
  grn_obj *database;
  bool closed;

  public:
    static void Initialize(v8::Handle<v8::Object> target);

    struct Baton {
      uv_work_t request;
      Nan::Persistent<v8::Function> callback;
      int error;
      char *result;
      unsigned int result_length;

      std::string command;
      grn_ctx context;
      grn_obj *database;
    };

  protected:
    static void New(const Nan::FunctionCallbackInfo<v8::Value>& info);
    static void CommandString(const Nan::FunctionCallbackInfo<v8::Value>& info);
    static void CommandSyncString(const Nan::FunctionCallbackInfo<v8::Value>& info);
    static void Close(const Nan::FunctionCallbackInfo<v8::Value>& info);
    Database() : ObjectWrap() {
    }
    bool Cleanup();
    ~Database() {
      if (!closed) {
        assert(Cleanup());
      }
    }
    static void CommandWork(uv_work_t* req);
    static void CommandAfter(uv_work_t* req);
};

void InitNroonga(v8::Local<v8::Object> exports);

} // namespace nroonga
#endif
