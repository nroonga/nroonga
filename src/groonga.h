#ifndef NROONGA_GROONGA_H
#define NROONGA_GROONGA_H
#include <node.h>
#include <node_object_wrap.h>
#include <v8.h>
#include <groonga/groonga.h>

#include <string>

using namespace v8;
using namespace node;

namespace node_groonga {

class Database : ObjectWrap {
  grn_ctx context;
  grn_obj *database;
  public:
    static void Initialize(v8::Handle<v8::Object> target);

    struct Baton {
      uv_work_t request;
      Persistent<Function> callback;
      int error;
      std::string message;
      char *result;
      unsigned int result_length;

      char *command;
      int command_length;
      grn_ctx context;
      grn_obj *database;
    };

  protected:
    static v8::Handle<v8::Value> New(const v8::Arguments& args);
    static v8::Handle<v8::Value> Command(const v8::Arguments& args);
    static v8::Handle<v8::Value> CommandSync(const v8::Arguments& args);
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

void InitGroonga(v8::Handle<v8::Object> target);

} // namespace node_groonga
#endif
