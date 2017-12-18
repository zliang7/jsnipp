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

#include <cassert>

#include "jsobject.h"
#include "jsarray.h"

namespace jsni {

class JSFunction : public JSObject {
public:
    JSFunction(JSValueRef jsval): JSObject(jsval, true) {
        assert(check(*this));
    }

    // create empty function
    constexpr JSFunction(std::nullptr_t = nullptr):
        JSObject(nullptr, true) {}
    JSFunction(const std::string& arguments, const std::string& body);

    std::string name() const {
        assert(*this);
        return getProperty("name", String);
    }

    JSValue apply(JSValue self, JSArray args) const;
    template <typename... Ts>
    JSValue call(JSValue self, Ts&&... args) const {
        return apply(self, JSArray(sizeof...(Ts), std::forward<Ts>(args)...));
    }
    template <typename... Ts>
    JSValue operator()(Ts&&... args) const {
        return call(nullptr, std::forward<Ts>(args)...);
    }

    static bool check(JSValueRef jsval) {
        return JSNIIsFunction(env, jsval);
    }
    static JSFunction from(JSValue jsval);

protected:
    typedef std::remove_pointer<JSNICallback>::type CallbackType;
    JSFunction(JSNICallback callback, bool save = false);
    operator JSNICallback() const;

    void setName(const std::string& name);
    friend class JSPropertyDescriptor;
};


// TODO: JSArguments instead of JSArray
using JSFunctionType = JSValue (*)(JSObject, JSArray);

template<JSFunctionType function>
class JSNativeFunction: public JSFunction {
public:
    JSNativeFunction(): JSFunction(thunk){}

    JSNativeFunction(const std::string& name): JSNativeFunction() {
        setName(name);
    }

private:
    static void thunk(JSNIEnv* env, const JSNICallbackInfo info);
};


template <class T>
using JSMethodType = JSValue (T::*)(JSObject, JSArray);

template <class T, JSMethodType<T> method>
class JSNativeMethod : public JSFunction {
public:
    JSNativeMethod(): JSFunction(thunk){}

    JSNativeMethod(const std::string& name): JSNativeMethod() {
        setName(name);
    }

private:
    static void thunk(JSNIEnv* env, const JSNICallbackInfo info);
};


template <class T>
using JSGetterType = JSValue (T::*)(JSObject);

template <class T, JSGetterType<T> getter>
class JSNativeGetter : public JSFunction {
public:
    JSNativeGetter(): JSFunction(thunk, true){}

private:
    static void thunk(JSNIEnv* env, const JSNICallbackInfo info);
};

template <class T>
using JSSetterType = void (T::*)(JSObject, JSValue);

template <class T, JSSetterType<T> setter>
class JSNativeSetter : public JSFunction {
public:
    JSNativeSetter(): JSFunction(thunk, true) {}

private:
    static void thunk(JSNIEnv* env, const JSNICallbackInfo info);
};

template <class T>
using JSAccessorType = JSValue (T::*)(JSObject, JSValue);

template <class T, JSAccessorType<T> accessor>
class JSNativeAccessor : public JSFunction {
public:
    JSNativeAccessor(): JSFunction(thunk, true){}

private:
    static void thunk(JSNIEnv* env, const JSNICallbackInfo info);
};

// FYI: http://stackoverflow.com/questions/15148749/pointer-to-class-member-as-a-template-parameter
// for template argument deduction.

}


#include "jsproperty.h"

namespace jsni {

inline JSFunction::JSFunction(const std::string& arguments,
                              const std::string& body):
    JSObject(constructor()["constructor"].as(Function)(arguments, body), true) {
    if (!is(Function))  jsval_ = nullptr;
}

inline JSValue JSFunction::apply(JSValue self, JSArray args) const {
    assert(*this);
    if (!*this) {
        // TODO: throw an error
        return JSUndefined();
    }

    size_t argc = args.length();
    JSValueRef jsvals[argc];
    for (size_t i = 0; i < argc; ++i)
        jsvals[i] = args[i];
    return JSNICallFunction(env, jsval_, self, argc, jsvals);
    /* an alternative implementation
    JSValueRef jsfunc = getProperty("apply");
    JSValueRef arg = args;
    return JSNICallFunction(env, jsfunc, self, 1, &arg);*/
}

inline JSFunction::JSFunction(JSNICallback callback, bool save):
    JSFunction(JSNINewFunction(env, callback)) {
    if (save) {
        auto jsobj = JSNativeObject<CallbackType>(callback, 0, nullptr);
        defineProperty("_jsni", JSPropertyDescriptor(jsobj, false));
    }
}

inline JSFunction::operator JSNICallback() const {
    if (*this) {
        auto jsni = getProperty("_jsni");
        if (jsni.is(Object))
            return JSNativeObject<CallbackType>(jsni).native();
    }
    return nullptr;
}

inline void JSFunction::setName(const std::string& name) {
    if (!*this)  return;
    auto desc = JSPropertyDescriptor(name, false, true);
    defineProperty("name", desc);
}

inline JSFunction JSFunction::from(JSValue jsval) {
    if (jsval.is(Function))  return jsval.as(Function);
    return JSFunction();
}

template <JSFunctionType function>
void JSNativeFunction<function>::thunk(JSNIEnv* env, const JSNICallbackInfo info) {
    assert(env == JSValue::env);
    JSObject self = JSNIGetThisOfCallback(env, info);
    JSValue result = (*function)(self, info);
    JSNISetReturnValue(env, info, result);
}

template <class T, JSMethodType<T> method>
void JSNativeMethod<T, method>::thunk(JSNIEnv* env, const JSNICallbackInfo info) {
    assert(env == JSValue::env);
    JSNativeObject<T> self(JSNIGetThisOfCallback(env, info));
    T* native = self.native();
    if (native) {
        JSValue result = (native->*method)(self, info);
        JSNISetReturnValue(env, info, result);
    } else {
        //TODO: throw an exception
    }
}

template <class T, JSGetterType<T> getter>
void JSNativeGetter<T, getter>::thunk(JSNIEnv* env, const JSNICallbackInfo info) {
    assert(env == JSValue::env);
    JSNativeObject<T> self(JSNIGetThisOfCallback(env, info));
    T* native = self.native();
    if (native) {
        JSValue result = (native->*getter)(self);
        JSNISetReturnValue(env, info, result);
    } else {
        //TODO: throw an exception
    }
}

template <class T, JSSetterType<T> setter>
void JSNativeSetter<T, setter>::thunk(JSNIEnv* env, const JSNICallbackInfo info) {
    assert(env == JSValue::env);
    JSNativeObject<T> self(JSNIGetThisOfCallback(env, info));
    T* native = self.native();
    if (native) {
        (native->*setter)(self, JSNIGetArgOfCallback(env, info, 0));
    } else {
        //TODO: throw an exception
    }
}

template <class T, JSAccessorType<T> accessor>
void JSNativeAccessor<T, accessor>::thunk(JSNIEnv* env,
                                          const JSNICallbackInfo info) {
    assert(env == JSValue::env);
    JSNativeObject<T> self(JSNIGetThisOfCallback(env, info));
    T* native = self.native();
    if (!native) {
        return;  //TODO: throw an exception
    }

    int argc = JSNIGetArgsLengthOfCallback(env, info);
    if (argc == 0) {
        JSValue result = (native->*accessor)(self, JSValue());
        JSNISetReturnValue(env, info, result);
    } else {
        assert(argc == 1);
        (native->*accessor)(self, JSNIGetArgOfCallback(env, info, 0));
    }
}

}
