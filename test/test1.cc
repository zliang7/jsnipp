#include "jsnipp.h"
#include <forward_list>
#include <map>

using namespace jsni;
int main() {
    JSNIEnv* env = nullptr;
    initialize(env);
    (void)version();

    JSValue jsval;
    JSValue jsval1;
    jsval.is(Function);
    jsval.is<JSFunction>();
    jsval.to(String);
    jsval.as(Number);
    (void)(jsval == jsval1);
    (void)(jsval != jsval1);

    // JSGlobalValue
    JSGlobalValue jsgval(jsval);
    JSGlobalValue jsgval1(jsval);
    JSGlobalValueRef g = nullptr;
    JSGlobalValue gg = g;
    (void)(jsgval == jsval1);
    (void)(jsgval != jsval1);

    JSNull null;
    JSUndefined undefined;

    // boolean
    JSBoolean b;
    JSBoolean b1(jsval, true);
    b = true;
    (void)(b == true);
    (void)(b != true);
    (void)(true == b);
    (void)(true != b);
    (void)!b;

    // number
    JSNumber num;
    JSNumber num1(jsval, 2.3);
    num = 100.0;
    num = 100;
    num += 100;
    num += num1;
    num++;
    num--;
    ++num;
    --num;
    (void)(num + 100.0);
    (void)(100 - num);
    (void)(num - 100);
    (void)(num == 100.0);
    (void)(100.0 != num);
    (void)(num == 100);
    (void)(100 == num);

    // string
    JSString str;
    JSString str1(jsval, "hello");
    str = "asdf";
    (void)(str == "asdf");
    (void)("asdf" == str);
    str += "xxxx";
    (void)(str + "xxxx");
    (void)(str.length() == 8);

    // object
    auto obj = JSObject();
    auto obj1 = JSObject{
        {"b", true},
        {"i", 1},
        {"d", 2.1},
        {"s", "str"},
        {"n", nullptr},
        {"o", JSObject{
          {"1", 1.0},
          {"2", false}
        }},
        {"a", JSArray{
          1, "2", 3.0, false, "str", nullptr
        }}
    };
    auto obj2 = JSObject(
        "b", true,
        "i", 1,
        "d", 2.1,
        "s", "str",
        "n", nullptr,
        "o", JSObject(
          "1", 1.0,
          "2", false
        ),
        "a", JSArray{
          1, "2", 3.0, false, "str", nullptr
        }
    );
    auto obj3 = JSObject(jsval, {"b", true});
    auto obj4 = JSObject(jsval, {});
    auto obj5 = JSObject(std::map<std::string, int>());
    (void)obj1["a"];
    obj1["a"] = 100.0;
    obj1["a"] = obj1["b"];

    // NativeObject
    struct _ST {};
    auto nobj = JSNativeObject<_ST>();

    // Array
    auto arr = JSArray{ "1", 2, nullptr };
    auto arr1 = JSArray(3, "1", 2, nullptr );
    auto arr2 = JSArray (std::forward_list<std::string>());
    (void)arr[1];
    arr.getElement(1);
    arr.getElement(1, Object);
    arr.getElement<JSObject>(1);
    arr[1] = 100;  // ?? effective?
    arr[1] = arr[2];

    // typedarray
    unsigned char buf[100];
    auto tarr = JSTypedArray<unsigned char>(buf, sizeof(buf));
    (void)tarr.length();
    (void)(tarr.buffer() == buf);
    auto tarr1 = JSTypedArray<float>();
    //auto tarr2 = JSTypedArray<char*>();

    // function
    auto jsfun = JSFunction(tarr);  // FIXME
    auto jsfun1 = JSFunction(jsval, nullptr);
    jsfun(nullptr, 1, 2.3, "asf", true);

    return 0;
}
