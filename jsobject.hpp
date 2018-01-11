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

#include <unordered_set>

#include "jsobject.h"
#include "jsproperty.h"
#include "jsfunction.h"

namespace jsni {

inline bool isKnownStaticMethod(const std::string& name) {
    static std::unordered_set<std::string> methodset {
        "create",
        "defineProperties",
        "defineProperty",
        "freeze",
        "getOwnPropertyDescriptor",
        "getOwnPropertyNames",
        "getPrototypeOf",
        "isExtensible",
        "isFrozen",
        "isSealed",
        "keys",
        "preventExtensions",
        "seal",
        // ECMAScript 2015 (6th Edition, ECMA-262)
        "assign",
        "getOwnPropertySymbols",
        "setPrototypeOf",
        // ECMAScript 2017 (ECMA-262)
        "entries",
        "getOwnPropertyDescriptors",
        "values"
    };
    return methodset.find(name) != methodset.end();
}

template <typename... Ts>
JSValue JSObject::callMethod(const std::string& name, Ts&&... args) {
    return isKnownStaticMethod(name) ?
            constructor()[name].as(Function)(*this, std::forward<Ts>(args)...) :
            getProperty(name, Function).call(*this, std::forward<Ts>(args)...);
}

inline bool JSObject::defineProperty(const std::string& name,
                                     const JSPropertyDescriptor& descriptor) {
    JSNIPropertyDescriptor desc = descriptor;
    if (desc.data_attributes || desc.accessor_attributes)
        return JSNIDefineProperty(env, jsval_, name.c_str(), desc);

    callMethod("defineProperty", name, descriptor);
    return true;
}

inline JSObject::JSObject(JSValueRef jsval): JSValue(jsval) {
    if (is(Null) || is(Undefined))
        jsval_ = JSObject();
    else if (!is(Object))
        jsval_ = constructor().as(Function)(jsval);
}


template <class T> template <JSMethodType<T> method>
bool JSNativeObject<T, typename std::enable_if<std::is_class<T>::value>::type>
                   ::defineMethod(const std::string& name) {
    const auto callback = JSNativeMethod<T, method>::thunk;
    return JSNIRegisterMethod(env(), this->jsval_, name.c_str(), callback);
}

template <class T> template <JSGetterType<T> getter, JSSetterType<T> setter>
bool JSNativeObject<T, typename std::enable_if<std::is_class<T>::value>::type>
                   ::defineProperty(const std::string& name) {
    JSNIAccessorPropertyDescriptor desc {
        JSNativeGetter<T, getter>::thunk,
        setter != nullptr ? JSNativeSetter<T, setter>::thunk : nullptr,
        JSNINone, nullptr
    };
    return JSNIDefineProperty(env(), this->jsval_, name.c_str(),
                              {nullptr, &desc});
}

template <class T> template <JSAccessorType<T> accessor>
bool JSNativeObject<T, typename std::enable_if<std::is_class<T>::value>::type>
                   ::defineProperty(const std::string& name) {
    JSNIAccessorPropertyDescriptor desc {
        JSNativeAccessor<T, accessor>::thunk,
        JSNativeAccessor<T, accessor>::thunk,
        JSNINone, nullptr
    };
    return JSNIDefineProperty(env(), this->jsval_, name.c_str(),
                              {nullptr, &desc});
}

/*
template <class T>
bool JSNativeObjectBase<T>::defineProperty(const std::string& name,
                                           JSValue& value) {
    JSNIDataPropertyDescriptor desc {value, JSNIReadOnly};
    return JSNIDefineProperty(env(), this->jsval_, name.c_str(),
                              {&desc, nullptr});
}
template <class T>
bool JSNativeObjectBase<T>::defineProperty(const std::string& name,
                                           const JSValue& value) {
    JSNIDataPropertyDescriptor desc {value, JSNINone};
    return JSNIDefineProperty(env(), this->jsval_, name.c_str(),
                              {&desc, nullptr});
}*/

}
