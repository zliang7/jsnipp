#include <jsnipp.h>

using namespace jsni;

/* jsni version:
void SayHello(JSNIEnv* env, const JSNICallbackInfo info) {
    JSValueRef jsstr = JSNINewStringFromUtf8(env,"Hello, World!", 14);
    JSNISetReturnValue(env, info, jsstr);
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
    // setter
    void set_prefix(JSObject, JSValue val) {
        prefix_ = val.to(String);
    }
    /* initializer
    static void setup(JSObject cls) {
        cls.defineProperty("echo", JSNativeMethod<Echo, &Echo::echo>());
        cls.defineProperty("prefix", JSNativeGetter<Echo, &Echo::prefix>());
        cls.defineProperty("prefix2", {JSNativeGetter<Echo, &Echo::prefix>(),
                                       JSNativeSetter<Echo, &Echo::set_prefix>()});
    }*/
    static void setup(JSNativeObject<Echo>& proto) {
        //cls.defineProperty("echo",  JSNativeMethod<Echo, &Echo::echo>());
        proto.defineMethod<&Echo::echo>("echo");
        proto.defineProperty<&Echo::prefix>("prefix");
        proto.defineProperty<&Echo::prefix, &Echo::set_prefix>("prefix2");
    }
private:
    std::string prefix_;
};

// JSNI Entry point
__attribute__ ((visibility("default")))
int JSNI_Init(JSNIEnv* env, JSValueRef exports) {
    //LOG_I("JSNI module is loaded");
    auto jsobj = initialize(env, exports);

    // register native function
    jsobj.setProperty("sayHello", JSNativeFunction<SayHello>());
    //JSNIRegisterMethod(env, exports, "sayHello", SayHello);

    // register native constructor
    jsobj.setProperty("Echo", JSNativeConstructor<Echo>("Echo", &Echo::setup));
    jsobj.setProperty("Echo", JSNativeConstructor<Echo>{
        {"echo", JSNativeMethod<Echo, &Echo::echo>()},
        {"string", "hello"},
    });

    /* register native object
    Test* native = new Test();
    obj.setProperty("object", JSNativeObject<Test>(native, {
        {"func", JSNativeMethod<Test, &Test::func>()},
        {"str", "hello"_js},
        {"num", 1.1_js},
        {"bool", true_js},
        {"nil", null_js}
    }));*/

    // register array
    //auto array = JSArray { 1.1, 2.2, 3.3 };
    auto array = JSArray(0, true, 1.1, "hello");
    jsobj.setProperty("anArray", array);

    // register object
    JSObject object{
        {"asdf", "asdf"},
        {"1234", 1.1}
    };
    //jsobj.setProperty("anObject", object);
    jsobj["anObject"] = object;
    JSValue x = jsobj["anObject"];
    x=jsobj["asdf"];
    //x(object);

    JSException::raise(JSException::TypeError, "asdf");

    auto ta = JSTypedArray<uint8_t, true>();
    ta.is(TypedArray);
    ta.is(Uint8ClampedArray);
    jsobj.setProperty("typed", ta);

    auto ao = JSAssociatedObject(5);
    (void)ao.get<char>(1);
    (void)ao.get<char*>(2);
    (void)ao.get<const char*>(3);
    ao.set(1, 100);
    char* p = nullptr;
    ao.set(2, p);
    ao.set(3, "asdf");
    ao.set(3, JSNI_Init);
    //JSBoolean aaa(ao); FIXME

    auto cb1 = JSCallback<void, int, int, int>();
    cb1(1, 2, 3);
    //JSCallback<void, bool> cb2(nullptr, nullptr);
    //cb2(true);

    //JSNativeObject<Echo>::set_compatible_types<int, bool, long>();
    return JSNI_VERSION_2_1;
}
