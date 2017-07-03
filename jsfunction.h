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

#include <jsni.h>

#include "jsobject.h"
#include "jsarray.h"

namespace jsnipp {

class JSFunction : public JSObject {
public:
    JSFunction(const JSValue& jsval): JSObject(jsval) {
        if (!is_function())  jsval_ = JSNull();
    }
    JSFunction(JsValue jsval): JSObject(jsval) {
        assert(is_function());
    }

    // null function
    JSFunction(std::nullptr_t null = nullptr):
        JSObject(null) {}

    operator bool() const {
        return is_function();
    }

    std::string name() const {
        return JSString(getProperty("name"));
    }

    JSValue apply(JSObject self, JSArray args) const;
    template <typename... Ts>
    JSValue call(JSObject self, Ts... args) const {
        return apply(self, JSArray(sizeof...(Ts), args...));
    }
    template <typename... Ts>
    JSValue operator()(Ts... args) const {
        return call(nullptr, args...);
    }

protected:
    JSFunction(NativeFunctionCallback callback):
        JSObject(env_->NewFunction(callback)) {}

    void setName(const std::string& name);
};


using JSFunctionType = JSValue (*)(JSObject, JSArray);

template<JSFunctionType function>
class JSNativeFunction: public JSFunction {
public:
    JSNativeFunction():
        JSFunction([](JSNIEnv* env, const CallbackInfo info){
            assert(env == env_);
            JSObject self = env->GetThis(info);
            JsValue result = (*function)(self, info);
            env->SetReturnValue(info, result);
        }){}
};

template <class C>
using JSMethodType = JSValue (C::*)(JSObject, JSArray);

template <class C, JSMethodType<C> method>
class JSNativeMethod : public JSFunction {
public:
    JSNativeMethod() :
        JSFunction([](JSNIEnv* env, const CallbackInfo info){
            assert(env == env_);
            JSNativeObject<C> self(env->GetThis(info));
            C* native = self.native();
            if (native) {
                JsValue result = (native->*method)(self, info);
                env->SetReturnValue(info, result);
            } else {
                //TODO: throw an exception
            }
        }){}
};

template <class C>
using JSGetterType = JSValue (C::*)(JSObject);

template <class C, JSGetterType<C> getter>
class JSNativeGetter : public JSFunction {
public:
    JSNativeGetter() :
        JSFunction([](JSNIEnv* env, const CallbackInfo info){
            assert(env == env_);
            JSNativeObject<C> self(env->GetThis(info));
            C* native = self.native();
            if (native) {
                env->SetReturnValue(info, (native->*getter)(self));
            } else {
                //TODO: throw an exception
            }
        }){}
};

template <class C>
using JSSetterType = void (C::*)(JSObject, JSValue);

template <class C, JSSetterType<C> setter>
class JSNativeSetter : public JSFunction {
public:
    JSNativeSetter() :
        JSFunction([](JSNIEnv* env, const CallbackInfo info){
            assert(env == env_);
            JSNativeObject<C> self(env->GetThis(info));
            C* native = self.native();
            if (native) {
                (native->*setter)(self, env->GetArg(info, 0));
            } else {
                //TODO: throw an exception
            }
        }){}
};

// FYI: http://stackoverflow.com/questions/15148749/pointer-to-class-member-as-a-template-parameter
// for template argument deduction.

}


#include "jsproperty.h"

namespace jsnipp {

inline JSValue JSFunction::apply(JSObject self, JSArray args) const {
    size_t argc = args.length();
    JsValue jsvals[argc];
    for (size_t i = 0; i < argc; ++i)
        jsvals[i] = args[i];
    return from(env_->CallFunction(jsval_, self, argc, jsvals));
    /* an alternative implementation
    JSFunction jsapply = getProperty("apply");
    JsValue arg = args;
    return from(env_->CallFunction(jsapply, self, 1, &arg));*/
}

inline void JSFunction::setName(const std::string& name) {
    defineProperty("name", JSPropertyData(JSString(name), false, true));
}

}
