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

#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>

#include "jsvalue.h"

namespace jsni {

class JSPropertyDescriptor;
class JSFunction;

class JSObject : public JSValue {
public:
    JSObject(JSValueRef jsval);
    explicit JSObject(JSValueRef jsval, JSObject defval):
        JSValue(jsval) {
        if (!check(jsval_))  jsval_ = defval;
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
    JSObject(const std::string& name, JSValue value, Ts&&... args):
        JSObject(std::forward<Ts>(args)...) {
        setProperty(name, value);
    }

    JSObject prototype() const {
        return JSObject(NoCheck(JSNIGetPrototype(env, jsval_)));
    }
    void setPrototype(JSObject proto) {
        callMethod("setPrototypeOf", proto);
    }
    bool isPrototypeOf(JSObject object) const {
        auto self = const_cast<JSObject*>(this);
        return self->callMethod("isPrototypeOf", object).as(Boolean);
    }

    std::string toString() const {
        auto self = const_cast<JSObject*>(this);
        return self->callMethod("toString").as(String);
    }

    // TODO: support Symbol
    template <typename T>
    T getProperty(const std::string& name, JSTypeID<T> = JSTypeID<T>()) const {
        return getProperty(name).to<T>();
    }
    JSValue getProperty(const std::string& name) const {
        return JSNIGetProperty(env, jsval_, name.c_str());
    }
    bool setProperty(const std::string& name, JSValue jsval) {
        return JSNISetProperty(env, jsval_, name.c_str(), jsval);
    }
    bool hasProperty(const std::string& name) const {
        return JSNIHasProperty(env, jsval_, name.c_str());
    }
    bool deleteProperty(const std::string& name) {
        return JSNIDeleteProperty(env,jsval_, name.c_str());
    }
    bool defineProperty(const std::string& name,
                        const JSPropertyDescriptor& descriptor);

    template <typename... Ts>
    JSValue callMethod(const std::string& name, Ts&&... args);

/*  class Accessor final {
    public:
        operator JSValue() const {
            return obj_.getProperty(name_);
        }
        Accessor& operator=(JSValue value) {
            obj_.setProperty(name_, value);
            return *this;
        }
        Accessor& operator=(const Accessor& ref) {
            obj_.setProperty(name_, ref);
            return *this;
        }

    private:
        Accessor(const Accessor&) = default;
        Accessor(JSObject& obj, const std::string& name):
            obj_(obj), name_(name){}

        const std::string& name_;
        JSObject& obj_;
        friend class JSObject;
    };*/
    JSValue operator [](const std::string& name) const {
        return getProperty(name);
    }
    JSValue operator [](const char* name) const {
        return getProperty(name);
    }
/*  Accessor operator [](const std::string& name) {
        return Accessor(*this, name);
    }*/

    static bool check(JSValueRef jsval) {
        return JSNIIsObject(env, jsval);
    }

protected:
    struct NoCheck {
        constexpr explicit NoCheck(JSValueRef jsval): val_(jsval){}
        constexpr operator JSValueRef() const { return val_; }
        JSValueRef val_;
    };
    constexpr JSObject(NoCheck jsval): JSValue(jsval) {}
    explicit JSObject(int cnt):
        JSValue(JSNINewObjectWithInternalField(env, cnt)) {}

    static JSObject constructor() {
        return JSObject().getProperty("constructor").as(Object);
    }
    friend class JSValue;
};


template<class T>
class JSNativeObject : public JSObject {
public:
    JSNativeObject(T* native, unsigned int count = 0,
                   std::function<void(T*)> deleter = std::default_delete<T>());
    JSNativeObject(std::nullptr_t = nullptr): JSNativeObject(nullptr, 0) {}

/*  JSNativeObject(T* native, initializer_list list): JSNativeObject(native) {
        for (auto&& p: list)
            setProperty(p.first, p.second);
    }*/

    JSNativeObject(JSValueRef jsval);

    T* operator->() const noexcept {
        return this->native();
    }
    T& operator*() const noexcept {
        return *this->native();
    }

    T* native() const {
        return reinterpret_cast<T*>(getPrivate(count()));
    }
    T* reset(T* native);

private:
    int count() const {
        return JSNIInternalFieldCount(env, jsval_) - 2;
    }
    void* getPrivate(int index) const {
        return JSNIGetInternalField(env, jsval_, index);
    }
    void setPrivate(int index, void* ptr) {
        return JSNISetInternalField(env, jsval_, index, ptr);
    }
};

}


#include <cassert>
#if (__cpp_rtti || defined(__GXX_RTTI)) && !defined(NDEBUG)
#include <typeinfo>
#endif

namespace jsni {

template<class T>
JSNativeObject<T>::JSNativeObject(T* native, unsigned int count,
                                  std::function<void(T*)> deleter):
    JSObject(count + 2) {
    reset(native);
    if (deleter)
        JSGlobalValue(jsval_).setGCCallback(std::bind(deleter, native));

#if (__cpp_rtti || defined(__GXX_RTTI)) && !defined(NDEBUG)
    auto hash = static_cast<uintptr_t>(typeid(T).hash_code());
    setPrivate(count + 1, reinterpret_cast<void*>(hash));
#endif
}

template<class T>
JSNativeObject<T>::JSNativeObject(JSValueRef jsval): JSObject(jsval) {
#if (__cpp_rtti || defined(__GXX_RTTI)) && !defined(NDEBUG)
    // check native type
    int index = count() + 1;
    assert(index > 0);
    if (!std::is_class<T>::value) {
        // FIXME: deal with derived class
        auto hash = reinterpret_cast<uintptr_t>(getPrivate(index));
        assert(typeid(T).hash_code() == static_cast<size_t>(hash));
    }
#endif
}

template<class T>
T* JSNativeObject<T>::reset(T* native) {
    int index = count();
    T* old = reinterpret_cast<T*>(getPrivate(index));
    if (native == old)  return nullptr;
    setPrivate(index, reinterpret_cast<void*>(native));
    return old;
}

}
