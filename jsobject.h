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

#include <algorithm>
#include <cassert>
#include <functional>
#include <initializer_list>
#include <memory>
#include <unordered_set>
#include <string>
#include <utility>

#if (__cpp_rtti || defined(__GXX_RTTI)) && !defined(NDEBUG)
#include <typeinfo>
#define CHECK_NATIVE_TYPE
#endif

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

    static JSObject constructor() {
        return JSObject().getProperty("constructor").as(Object);
    }
    friend class JSValue;
};


class JSAssociatedObject : public JSObject {
    // type traits
    template <typename T>
    struct is_const_pointer : std::integral_constant<bool,
        std::is_pointer<T>::value && std::is_const<
            typename std::remove_pointer<T>::type>::value>{};
    template <typename T>
    struct is_nonconst_pointer : std::integral_constant<bool,
        std::is_pointer<T>::value && !std::is_const<
            typename std::remove_pointer<T>::type>::value>{};

public:
    JSAssociatedObject(JSValueRef jsval): JSObject(jsval){}

    template <typename... Ts>
    explicit JSAssociatedObject(size_t count, Ts... args):
        JSObject(NoCheck(JSNINewObjectWithInternalField(
                    env, std::max(count, sizeof...(Ts))))) {
        reduce_args(sizeof...(Ts), args...);
    }

/*  void setGCCallback(std::function<void(JSValue)> deleter) {
        auto jsgval = JSGlobalValue(*this);
        JSGlobalValueRef ref = jsgval;
        jsgval.setGCCallback([=](){
            deleter(JSNIGetGlobalValue(env, ref));
        });
    }*/

    int count() const {
        return JSNIInternalFieldCount(env, jsval_);
    }
    explicit operator bool() const {
        return jsval_ && count() > 0;
    }

    template <typename T>
    typename std::enable_if<!std::is_pointer<T>::value, T>::type
    get(int index) const {
        assert(index >= count());
        auto ptr = JSNIGetInternalField(env, jsval_, index);
        return static_cast<T>(reinterpret_cast<uintptr_t>(ptr));
    }
    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value, T>::type
    get(int index) const {
        assert(index >= count());
        auto ptr = JSNIGetInternalField(env, jsval_, index);
        return reinterpret_cast<T>(ptr);
    }

    template <typename T>
    typename std::enable_if<!std::is_pointer<T>::value>::type
    set(int index, T val) {
        assert(index >= count());
        auto ptr = static_cast<uintptr_t>(val);
        JSNISetInternalField(env, jsval_, index, reinterpret_cast<void*>(ptr));
    }
    template <typename T>
    typename std::enable_if<is_nonconst_pointer<T>::value>::type
    set(int index, T ptr) {
        assert(index >= count());
        JSNISetInternalField(env, jsval_, index, reinterpret_cast<void*>(ptr));
    }
    template <typename T>
    typename std::enable_if<is_const_pointer<T>::value>::type
    set(int index, T ptr) {
        assert(index >= count());
        typedef typename std::remove_pointer<T>::type U;
        auto p = const_cast<typename std::remove_const<U>::type*>(ptr);
        JSNISetInternalField(env, jsval_, index, reinterpret_cast<void*>(p));
    }

protected:
    JSAssociatedObject(NoCheck jsval): JSObject(jsval) {}

private:
    void reduce_args(size_t count) {}
    template <typename T, typename... Ts>
    void reduce_args(size_t count, T first, Ts... args) {
        size_t index = count - sizeof...(Ts) - 1;
        set(index, first);
        reduce_args(count, args...);
    }
};


template<class T>
class JSNativeObjectBase : public JSAssociatedObject {
public:
    explicit operator bool() const {
        return jsval_ && native();
    }

    T* native() const {
        return get<T*>(count() - 2);
    }
    T* reset(T* native) {
        int index = count() - 2;
        auto old = get<T*>(index);
        if (native == old)  return nullptr;
        set(index, native);
        return old;
    }

protected:
    JSNativeObjectBase(JSValueRef jsval):
        JSAssociatedObject(NoCheck(jsval_)) {}

    JSNativeObjectBase(T* native, unsigned int count,
                       std::function<void(T*)> deleter):
        JSAssociatedObject(count + 2) {
        reset(native);
        if (deleter)
            JSGlobalValue(jsval_).setGCCallback(std::bind(deleter, native));

#ifdef CHECK_NATIVE_TYPE
        set(count + 1, typeid(T).hash_code());
#endif
    }

    static bool check(JSValueRef jsval) {
        return JSObject::check(jsval) &&
               JSAssociatedObject(jsval).count() > 1;
    }

#ifdef CHECK_NATIVE_TYPE
    size_t hash_code() const {
        return get<size_t>(count() - 1);
    }
#endif

    template <typename, typename>
    friend class JSNativeObject;
};

template<class T, typename = void>
class JSNativeObject : public JSNativeObjectBase<T> {
public:
    JSNativeObject(JSValueRef jsval): JSNativeObjectBase<T>(jsval) {
        if (!check(this->jsval_))  this->jsval_ = JSNativeObject();
    }

    JSNativeObject(T* native, unsigned int count = 0,
                   std::function<void(T*)> deleter = nullptr):
        JSNativeObjectBase<T>(native, count, deleter) {}
    JSNativeObject(std::nullptr_t = nullptr):
        JSNativeObject(nullptr, 0) {};

    static bool check(JSValueRef jsval) {
        if (!JSNativeObjectBase<T>::check(jsval))
            return false;
#ifdef CHECK_NATIVE_TYPE
        auto jsobj = JSNativeObjectBase<T>(jsval);
        return jsobj.hash_code() == typeid(T).hash_code();
#else
        return true;
#endif
    }
};

// TODO: move this to jstypes.h
template <class T>
using JSMethodType = JSValue (T::*)(JSObject, JSArray);
template <class T>
using JSGetterType = JSValue (T::*)(JSObject);
template <class T>
using JSSetterType = void (T::*)(JSObject, JSValue);
template <class T>
using JSAccessorType = JSValue (T::*)(JSObject, JSValue);

template<class T>
class JSNativeObject<T, typename std::enable_if<std::is_class<T>::value>::type>
                     : public JSNativeObjectBase<T> {
public:
    JSNativeObject(JSValueRef jsval): JSNativeObjectBase<T>(jsval) {
        if (!check(this->jsval_))  this->jsval_ = JSNativeObject();
    }

    JSNativeObject(T* native, unsigned int count = 0,
                   std::function<void(T*)> deleter = std::default_delete<T>()):
        JSNativeObjectBase<T>(native, count, deleter) {}
    JSNativeObject(std::nullptr_t = nullptr):
        JSNativeObject(nullptr, 0) {};

    T* operator->() const noexcept {
        return this->native();
    }
    T& operator*() const noexcept {
        return *this->native();
    }

    template <JSMethodType<T> method>
    bool defineMethod(const std::string& name);
    template <JSGetterType<T> getter, JSSetterType<T> setter = nullptr>
    bool defineProperty(const std::string& name);
    template <JSAccessorType<T> accessor>
    bool defineProperty(const std::string& name);

    static bool check(JSValueRef jsval) {
        if (!JSNativeObjectBase<T>::check(jsval))
            return false;
#ifdef CHECK_NATIVE_TYPE
        auto jsobj = JSNativeObjectBase<T>(jsval);
        if (std::is_polymorphic<T>::value)
            return dynamic_cast<T*>(jsobj.native()) != nullptr;
#if __cplusplus >= 201402L
        if (std::is_final<T>::value)
            return jsobj.hash_code() == typeid(T).hash_code();
#endif
#endif
        // TODO: support derived non-polymorphic class
        return true;
    }

#if 0//def CHECK_NATIVE_TYPE
    // for type checking
    template <typename U0, typename U1, typename... Us>
    static void add_compatible_types() {
        add_compatible_types<U0>();
        add_compatible_types<U1, Us...>();
    }
    template <typename U>
    static void add_compatible_types() {
        compatible_types.insert(std::addressof(typeid(U)));
    }
private:
    static std::unordered_set<const std::type_info*> compatible_types;
#endif
};
#if 0//def CHECK_NATIVE_TYPE
template<class T> std::unordered_set<const std::type_info*>
JSNativeObject<T, typename std::enable_if<std::is_class<T>::value>::type>::compatible_types;
#endif

}
