/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#pragma once

#include <string.h>

#include <cassert>
#include <functional>
#include <initializer_list>
#include <string>
#include <type_traits>

#include <jsni.h>

namespace jsni {

namespace internal {

template <typename Dummy>
struct _JSGlobalEnvironment {
    static JSNIEnv* env;
};
template <typename Dummy>
JSNIEnv* _JSGlobalEnvironment<Dummy>::env = NULL;

typedef _JSGlobalEnvironment<void> JSGlobalEnvironment;

}

inline JSNIEnv* env() {
    return internal::JSGlobalEnvironment::env;
}
inline int version() {
    return JSNIGetVersion(env());
}


class JSValue;
class JSUndefined;
class JSNull;
class JSBoolean;
class JSNumber;
class JSString;
class JSSymbol;
class JSObject;
class JSFunction;
class JSArray;
template <typename, bool> class JSTypedArray;

template <typename T> struct JSTypeID {
    typedef T type;
};
constexpr JSTypeID<JSValue>     Valid{};
constexpr JSTypeID<JSUndefined> Undefined{};
constexpr JSTypeID<JSNull>      Null{};
constexpr JSTypeID<JSBoolean>   Boolean{};
constexpr JSTypeID<JSNumber>    Number{};
constexpr JSTypeID<JSString>    String{};
constexpr JSTypeID<JSSymbol>    Symbol{};
constexpr JSTypeID<JSObject>    Object{};
constexpr JSTypeID<JSFunction>  Function{};
constexpr JSTypeID<JSArray>     Array{};

constexpr JSTypeID<JSTypedArray<void, false>>     TypedArray{};
constexpr JSTypeID<JSTypedArray<int8_t, false>>   Int8Array{};
constexpr JSTypeID<JSTypedArray<uint8_t, false>>  Uint8Array{};
constexpr JSTypeID<JSTypedArray<uint8_t, true>>   Uint8ClampedArray{};
constexpr JSTypeID<JSTypedArray<int16_t, false>>  Int16Array{};
constexpr JSTypeID<JSTypedArray<uint16_t, false>> Uint16Array{};
constexpr JSTypeID<JSTypedArray<int32_t, false>>  Int32Array{};
constexpr JSTypeID<JSTypedArray<uint32_t, false>> Uint32Array{};
constexpr JSTypeID<JSTypedArray<float, false>>    Float32Array{};
constexpr JSTypeID<JSTypedArray<double, false>>   Float64Array{};

class JSValue : protected internal::JSGlobalEnvironment {
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
    JSValue(const char* cstring):
        jsval_(JSNINewStringFromUtf8(env, cstring, strlen(cstring))) {}

    // converting constructors of object
    JSValue(std::initializer_list<std::pair<std::string, JSValue>> list):
        jsval_(JSNINewObject(env)) {
        for (auto&& p : list)
            JSNISetProperty(env, jsval_, p.first.c_str(), p.second);
    }
    JSValue(std::initializer_list<JSValue> list):
        jsval_(JSNINewArray(env, list.size())) {
        for (auto i = list.begin(); i < list.end(); ++i)
            JSNISetArrayElement(env, jsval_, i - list.begin(), *i);
    }

    constexpr explicit operator bool() const {
        return jsval_ != nullptr;
    }

    constexpr operator JSValueRef() const {
        return jsval_;
    }

    bool operator == (const JSValue& that) const;
    bool operator != (const JSValue& that) const {
        return !(*this == that);
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
};

#if 1

class JSGlobalValue {
public:
    // construct with raw JSGlobalRef value
    constexpr JSGlobalValue(std::nullptr_t = nullptr): jsgval_(nullptr) {}
    JSGlobalValue(JSGlobalValueRef jsgval): jsgval_(jsgval) {
        if (jsgval_)  JSNIAcquireGlobalValue(env(), jsgval_);
    }
    JSGlobalValue(JSGlobalValueRef&& jsgval): jsgval_(jsgval) {
        jsgval = nullptr;
    }
    ~JSGlobalValue() {
        if (jsgval_)  JSNIReleaseGlobalValue(env(), jsgval_);
    }

    // copy / move constructors
    JSGlobalValue(const JSGlobalValue& that):
        JSGlobalValue(that.jsgval_) {}
    JSGlobalValue(JSGlobalValue&& that): jsgval_(that.jsgval_) {
        that.jsgval_ = nullptr;
    }

    // converting constructor for local value
    explicit JSGlobalValue(const JSValue& jsval):
        jsgval_(JSNINewGlobalValue(env(), jsval)) {}

    JSGlobalValue& operator =(const JSGlobalValue& that) {
        if (jsgval_)  JSNIReleaseGlobalValue(env(), jsgval_);
        jsgval_ = that.jsgval_;
        if (jsgval_)  JSNIAcquireGlobalValue(env(), jsgval_);
        return *this;
    }
    JSGlobalValue& operator =(JSGlobalValue&& that) {
        if (jsgval_)  JSNIReleaseGlobalValue(env(), jsgval_);
        jsgval_ = that.jsgval_;
        that.jsgval_ = nullptr;
        return *this;
    }

    explicit operator bool() const {
        return jsgval_ != nullptr;
    }

    // convert to local value
    template <typename T, typename = typename
              std::enable_if<std::is_base_of<JSValue, T>::value>::type>
    operator T() const {
        assert(jsgval_);
        return T(JSNIGetGlobalValue(env(), jsgval_));
    }

    // get raw value
    operator JSGlobalValueRef() const {
        return jsgval_;
    }

    // compare with JSValue
    bool operator ==(const JSValue& that) const {
        return that == *this;
    }
    bool operator !=(const JSValue& that) const {
        return that != *this;
    }

    void setGCCallback(const std::function<void()>& callback) {
        assert(*this && callback);
        auto data = new std::function<void()>(callback);
        JSNISetGCCallback(env(), jsgval_, data, [](JSNIEnv*, void* data){
            auto callback = reinterpret_cast<std::function<void()>*>(data);
            (*callback)();
            delete callback;
        });
    }
    //void setGCCallback(const std::function<void(JSValue)>& callback);

private:
    JSGlobalValueRef jsgval_;
};

#else

using _JSGlobalValue =
    std::shared_ptr<std::remove_pointer<JSGlobalValueRef>::type>;
class JSGlobalValue: _JSGlobalValue {
public:
    JSGlobalValue(std::nullptr_t = nullptr):
        _JSGlobalValue(nullptr, Deleter()) {}
    JSGlobalValue(JSGlobalValueRef jsgval):
        _JSGlobalValue(jsgval, Deleter()) {}

    // copy / move constructors
    JSGlobalValue(const JSGlobalValue& that):
        _JSGlobalValue(that) {};
    JSGlobalValue(JSGlobalValue&& that):
        _JSGlobalValue(that) {};

    // converting constructor for local value
    explicit JSGlobalValue(JSValue jsval):
        JSGlobalValue(JSNINewGlobalValue(env(), jsval)) {}

    JSGlobalValue& operator =(const JSGlobalValue& that) {
        _JSGlobalValue::operator=(that);
        return *this;
    }
    JSGlobalValue& operator =(JSGlobalValue&& that) {
        _JSGlobalValue::operator=(that);
        return *this;
    }

    explicit operator bool() const {
        return _JSGlobalValue::operator bool();
    }

    // convert to local value
    template <typename T, typename = typename
              std::enable_if<std::is_base_of<JSValue, T>::value>::type>
    operator T() const {
        assert(get());
        return T(JSNIGetGlobalValue(env(), get()));
    }

    // get raw value
    operator JSGlobalValueRef() const {
        return get();
    }

    // compare with JSValue
    bool operator ==(const JSValue& that) const {
        return that == *this;
    }
    bool operator !=(const JSValue& that) const {
        return that != *this;
    }

    void setGCCallback(const std::function<void()>& callback) {
        assert(get() && callback);
        auto data = new std::function<void()>(callback);
        JSNISetGCCallback(env(), get(), data, [](JSNIEnv*, void* data){
            auto callback = reinterpret_cast<std::function<void()>*>(data);
            (*callback)();
            delete callback;
        });
    }

private:
    struct Deleter {
        void operator()(JSGlobalValueRef jsgval) const {
            if (jsgval) JSNIDeleteGlobalValue(env(), jsgval);
        }
    };
};
#endif

class JSScope {
public:
    JSScope(): slot_() {
        JSNIPushLocalScope(env());
    }
    ~JSScope() {
        JSNIPopLocalScope(env());
    }
    JSValue& escape(JSValue jsval) {
        return slot_ = jsval;
    }
private:
    JSValue slot_;
};


/* NOT tested
void JSGlobalValue::setGCCallback(const std::function<void(JSValue)>& callback) {
    assert(*this && callback);
    JSGlobalValueRef jsgval = jsgval_;
    auto data = new std::function<void(JSNIEnv* env)>(
        [callback, jsgval](JSNIEnv* env) {
            callback(JSNIGetGlobalValue(env, jsgval));
    });
    JSNISetGCCallback(env(), jsgval_, data, [](JSNIEnv* env, void* data) {
        auto callback = reinterpret_cast<std::function<void(JSNIEnv*)>*>(data);
        (*callback)(env);
        delete callback;
    });
}*/

}
