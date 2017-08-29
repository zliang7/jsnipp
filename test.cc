#include "jsnipp.h"

using namespace jsnipp;

/* jsni version:
void SayHello(JSNIEnv* env, CallbackInfo info) {
    JsValue js_str = env->NewStringFromUtf8("Hello, World!", 14);
    env->SetReturnValue(info, js_str);
}*/
// jsnipp version:
JSValue SayHello(JSObject, JSArray args) {
    return JSString("Hello, world");
}

class Echo {
public:
    // constuctor
    Echo(JSObject, JSArray args) {
        if (args.length() > 0)
            prefix_ = JSString(args[0]);
        else
            prefix_ = "???: ";
    }
    // method
    JSValue echo(JSObject, JSArray args) {
        std::string str;
        if (args.length() > 0)
            str = JSString(args[0]);
        else
            str = "";
        return JSString(prefix_ + str);
    }
    // getter
    JSValue prefix(JSObject) {
        return JSString(prefix_);
    }
    // initializer
    static std::string setup(JSObject cls) {
        cls.setProperty("echo", JSNativeMethod<Echo, &Echo::echo>());
        cls.defineProperty("prefix", JSPropertyAccessor(JSNativeGetter<Echo, &Echo::prefix>()));
        return "Echo";
    }
private:
    std::string prefix_;
};

// JSNI Entry point
__attribute__ ((visibility("default")))
int JSNI_Init(JSNIEnv* env, JsValue exports) {
    //LOG_I("JSNI module is loaded");
    JSValue::setup(env);
    JSObject jsobj(exports);

    // register native function
    jsobj.setProperty("sayHello", JSNativeFunction<SayHello>());
    //env->RegisterMethod(exports, "sayHello", SayHello);

    // register native constructor
    jsobj.setProperty("Echo", JSNativeConstructor<Echo>(&Echo::setup));
/*  obj.setProperty("Echo2", JSNativeConstructor<Echo>({
        {"echo", JSNativeMethod<Echo, &Echo::echo>()},
        {"string", "hello"_js},
    }));

    // register native object
    Test* native = new Test();
    obj.setProperty("object", JSNativeObject<Test>(native, {
        {"func", JSNativeMethod<Test, &Test::func>()},
        {"str", "hello"_js},
        {"num", 1.1_js},
        {"bool", true_js},
        {"nil", null_js}
    }));

    // register array
    //auto array = JSArray { 1.1, 2.2, 3.3 };
    auto array = JSArray(0, true, 1.1, "hello");
    obj.setProperty("anArray", array);

    // register object
    JSObject object{
        {"asdf", "asdf"_js},
        {"1234", 1.1_js}
    };
    //obj.setProperty("anObject", object);
    obj["anObject"] = object;
    JSValue x = obj["anObject"];
    x=obj["asdf"];
    //x(object);

    JSException<Error>("asdf");
    JSException<TypeError>::checkAndClear();
*/
    jsobj.setProperty("typed", JSTypedArray<uint8_t, true>());
    return JSNI_VERSION_1_1;
}
