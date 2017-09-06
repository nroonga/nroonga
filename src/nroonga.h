#ifndef NROONGA_GROONGA_H
#define NROONGA_GROONGA_H
#include <nan.h>
#include <uv.h>
#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>
#include <groonga.h>

#include <string>

using namespace v8;
using namespace node;

namespace nroonga {

class Database : public Nan::ObjectWrap {
  grn_ctx context;
  grn_obj *database;
  bool closed;

  public:
    static void Initialize(v8::Handle<v8::Object> target);

    struct Baton {
      uv_work_t request;
      Nan::Persistent<Function> callback;
      int error;
      char *result;
      unsigned int result_length;

      std::string command;
      grn_ctx context;
      grn_obj *database;
    };

  protected:
    static void New(const Nan::FunctionCallbackInfo<Value>& info);
    static void CommandString(const Nan::FunctionCallbackInfo<Value>& info);
    static void CommandSyncString(const Nan::FunctionCallbackInfo<Value>& info);
    static void Close(const Nan::FunctionCallbackInfo<Value>& info);
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

void InitNroonga(v8::Handle<v8::Object> target);

} // namespace nroonga
#endif
