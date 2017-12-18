#include <cassert>
#include <string.h>
#include <string>
#include <utility>
#include <initializer_list>

typedef const char* JSValueRef;
typedef void JSNIEnv;
JSNIEnv* env() { return nullptr; }
JSValueRef JSNINewUndefined(JSNIEnv*) { return "undefined"; }
JSValueRef JSNINewNull(JSNIEnv*) { return "null"; }
JSValueRef JSNINewBoolean(JSNIEnv*,bool b) { return b?"true":"false"; }
JSValueRef JSNINewNumber(JSNIEnv*,double d) { auto b = new char[32]; sprintf(b, "%f", d); return b; }
JSValueRef JSNINewStringFromUtf8(JSNIEnv*,const char* s, size_t) { return s; }
JSValueRef JSNINewObject(JSNIEnv*) { return "object"; }
JSValueRef JSNINewArray(JSNIEnv*, size_t s) { auto b = new char[32]; sprintf(b, "array[%zd]", s); return b; }
bool JSNIIsEmpty(JSNIEnv*,JSValueRef v) { return v = nullptr; }
bool JSNIIsUndefined(JSNIEnv*,JSValueRef v) { return !strcmp(v, "undefined"); }
bool JSNIIsNull(JSNIEnv*,JSValueRef v) { return !strcmp(v, "null"); }
bool JSNIIsObject(JSNIEnv*,JSValueRef v) { return !strcmp(v, "object"); }
bool JSNIIsArray(JSNIEnv*,JSValueRef v) { return !strncmp(v, "array[", 6); }

class JSValue;
class JSUndefined;
class JSNull;

template <typename T> struct JSTypeID {
    typedef T type;
};
constexpr JSTypeID<JSValue>     Valid;
constexpr JSTypeID<JSUndefined> Undefined;
constexpr JSTypeID<JSNull>      Null;

class JSValue {
public:
    // construct with raw JSValueRef value
    constexpr JSValue(): jsval_(nullptr) {}
    constexpr JSValue(JSValueRef jsval): jsval_(jsval) {}

    // converting constructors of primitive types
    JSValue(std::nullptr_t):
        jsval_(JSNINewNull(env)) {}
    JSValue(bool boolean):
        jsval_(JSNINewBoolean(env, boolean)) {}
    template <typename T, typename = typename
              std::enable_if<std::is_arithmetic<T>::value>::type>
    JSValue(T number):
        jsval_(JSNINewNumber(env, number)) {}
    JSValue(const std::string& string):
        jsval_(JSNINewStringFromUtf8(env, string.c_str(), string.length())) {}
/*  JSValue(const char* cstring):
        jsval_(JSNINewStringFromUtf8(env, cstring, strlen(cstring))) {}*/

    /* converting constructors of object
    JSValue(std::initializer_list<std::pair<std::string, JSValue>> list):
        jsval_(JSNINewObject(env)) {
        for (auto&& p : list)
            JSNISetProperty(env, jsval_, p.first.c_str(), p.second);
    }
    JSValue(std::initializer_list<JSValue> list):
        jsval_(JSNINewArray(env, list.size())) {
        for (auto i = list.begin(); i < list.end(); ++i)
            JSNISetArrayElement(env, jsval_, i - list.begin(), *i);
    }*/

    constexpr explicit operator bool() const {
        return jsval_ != nullptr;
    }

    constexpr operator JSValueRef() const {
        return jsval_;
    }

    // type checking, casting and converting
    template <typename T>
    constexpr bool is(JSTypeID<T> = JSTypeID<T>()) const {
        return T::check(*this);
    }
    template <typename T>
    constexpr T as(JSTypeID<T> = JSTypeID<T>()) const {
#if __cplusplus >= 201402L
        assert(is<T>());
#endif
        return *reinterpret_cast<const T*>(this);
    }
    template <typename T>
    constexpr T to(JSTypeID<T> = JSTypeID<T>()) const {
        return T::from(*this);
    }

    static bool check(JSValueRef jsval) {
        return !JSNIIsEmpty(env, jsval);
    }

protected:
    JSValueRef jsval_;
    static JSNIEnv* env;
};
JSNIEnv* JSValue::env = nullptr;

class JSObject : public JSValue {
public:
    JSObject(JSValueRef jsval): JSValue(jsval) {
        assert(check(*this));
    }

    JSObject(): JSValue(JSNINewObject(env)) {}

    using initializer_list =
            std::initializer_list<std::pair<std::string, JSValue>>;
    JSObject(initializer_list list): JSObject() {
        for (auto&& p: list)
            setProperty(p.first, p.second);
    }
    template <class T, typename = typename std::enable_if<
            std::is_constructible<std::string, typename T::key_type>::value &&
            std::is_constructible<JSValue, typename T::mapped_type>::value>::type>
    JSObject(const T& map): JSObject() {
        for (auto&& p: map)
            setProperty(p.first, p.second);
    }

    template <typename... Ts>
    JSObject(const std::string& name, JSValue value, Ts... args):
        JSObject(args...) {
        setProperty(name, value);
    }

    void setProperty(const std::string& name, JSValue value) {
        printf("%s: %s\n", name.c_str(), JSValueRef(value));
    }

    static bool check(JSValueRef jsval) {
        return JSNIIsObject(env, jsval);
    }

protected:
    constexpr JSObject(JSValueRef jsval, bool): JSValue(jsval) {}
};

class JSArray final : public JSObject {
public:
    JSArray(JSValueRef jsval): JSObject(jsval, true) {
        assert(check(*this));
    }

    explicit JSArray(size_t length = 0):
        JSObject(JSNINewArray(env, length), true) {}
    typedef std::initializer_list<JSValue> initializer_list;
    JSArray(initializer_list list): JSArray(list.size()) {
        for (auto p = list.begin(); p < list.end(); ++p)
            setElement(p - list.begin(), *p);
    }
    template <class T, typename = typename std::enable_if<
            std::is_constructible<JSValue, typename T::value_type>::value>::type>
    JSArray(const T& seq): JSArray(std::distance(seq.begin(), seq.end())) {
        size_t index = 0;
        for (auto it = seq.begin(); it != seq.end(); ++it)
            setElement(index++, *it);
    }

    template <typename... Ts>
    JSArray(size_t size, Ts... args): JSArray(std::max(size, sizeof...(Ts))) {
        reduce_args(sizeof...(Ts), args...);
    }
/*  template <typename... Ts>
    JSArray(size_t size, JSValue value, Ts... args):
        JSArray(std::max(size, sizeof...(Ts)), args...) {
        size_t index = size - sizeof...(Ts) - 1;
        setElement(index, value);
    }*/
    void setElement(size_t index, JSValue value) {
        printf("[%zd]: %s\n", index, JSValueRef(value));
    }

    static bool check(JSValueRef jsval) {
        return JSNIIsArray(env, jsval);
    }

private:
    void reduce_args(size_t length) {}
    template <typename T, typename... Ts>
    void reduce_args(size_t length, T first, Ts... args) {
        size_t index = length - sizeof...(Ts) - 1;
        setElement(index, first);
        reduce_args(length, args...);
    }
};

class JSUndefined final : public JSValue {
public:
    JSUndefined(JSValueRef jsval): JSValue(jsval) {
        assert(check(*this));
    }

    JSUndefined(): JSValue(JSNINewUndefined(env)) {}

    static bool check(JSValueRef jsval) {
        return JSNIIsUndefined(env, jsval);
    }
    static JSUndefined from(JSValue) {
        return JSUndefined();
    }
};
class JSNull final : public JSValue {
public:
    JSNull(JSValueRef jsval): JSValue(jsval) {
        assert(check(*this));
    }

    JSNull(std::nullptr_t = nullptr):
        JSValue(JSNINewNull(env)) {}

    static bool check(JSValueRef jsval) {
        return JSNIIsNull(env, jsval);
    }
    static JSNull from(JSValue) {
        return JSNull();
    }
};

int main() {
#if 1
    JSObject obj1 {
        {"b", true},
        {"i", 1},
        {"d", 2.1},
        {"s", "str"},
        {"n", nullptr},
        {"o", JSObject{
          {"1", 1.0},
          {"2", false}
        }},
        {"a", JSArray(
          8, "2", 3.0, false, "str", nullptr
        )}
    };
#else
    JSObject obj2(
        "b", true,
        "i", 1,
        "d", 2.1,
        "s", "str",
        "n", nullptr,
        "o", JSObject(
          "1", 1.0,
          "2", false
        )
     );
#endif
}
