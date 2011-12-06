#ifndef NROONGA_GROONGA_H
#define NROONGA_GROONGA_H
#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>
#include <groonga.h>

#include <string>

using namespace v8;
using namespace node;

namespace nroonga {

class Database : ObjectWrap {
  grn_ctx context;
  grn_obj *database;
  public:
    static void Initialize(v8::Handle<v8::Object> target);

    struct Baton {
      uv_work_t request;
      Persistent<Function> callback;
      int error;
      char *result;
      unsigned int result_length;

      std::string command;
      grn_ctx context;
      grn_obj *database;
    };

  protected:
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> CommandString(const v8::Arguments& args);
    static v8::Handle<v8::Value> CommandSyncString(const v8::Arguments& args);
    Database() : ObjectWrap() {
    }
    void CleanupDatabase() {
      grn_ctx_fin(&context);
    }
    ~Database() {
      CleanupDatabase();
    }
    static void CommandWork(uv_work_t* req);
    static void CommandAfter(uv_work_t* req);
};

void InitNroonga(v8::Handle<v8::Object> target);

} // namespace nroonga
#endif
