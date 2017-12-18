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

#include "jsfunction.h"

namespace jsni {

template <class T>
class JSNativeConstructor : public JSFunction {
public:
    // no prototype
    JSNativeConstructor(const std::string& name = ""): JSFunction(thunk) {
        setName(name);
    }

    // specified prototype
    JSNativeConstructor(const std::string& name, JSNativeObject<T> prototype);
    JSNativeConstructor(JSNativeObject<T> prototype):
        JSNativeConstructor("", prototype) {}

    // user-built prototype
    JSNativeConstructor(const std::string& name,
                        std::function<void(JSNativeObject<T>&)> builder):
        JSNativeConstructor(name, JSNativeObject<T>(nullptr)) {
        auto proto = JSNativeObject<T>(prototype_);
        builder(proto);
    }
    JSNativeConstructor(std::function<void(JSNativeObject<T>&)> builder):
        JSNativeConstructor("", builder) {}
    JSNativeConstructor(initializer_list list):
        JSNativeConstructor<T>([&list](JSNativeObject<T>& proto) {
            for (auto&& p: list) proto.setProperty(p.first, p.second);
        }) {}

    static JSNativeObject<T> wrap(T* native,
            std::function<void(T*)> deleter = std::default_delete<T>()) {
        JSNativeObject<T> jsobj(native, 0, deleter);
        if (prototype_)
            jsobj.setPrototype(prototype_);
        return jsobj;
    }

private:
    void setName(const std::string& name);

    static void thunk(JSNIEnv* env, const JSNICallbackInfo info);
    static JSGlobalValue prototype_;
};

}


//////////////////////////////////////////////////////////////////////////////

#include <cassert>
#if __cpp_rtti || __GXX_RTTI
#include <cxxabi.h>
#include <typeinfo>
#endif

namespace jsni {

template <class T>
JSGlobalValue JSNativeConstructor<T>::prototype_ = nullptr;

template <class T>
JSNativeConstructor<T>::JSNativeConstructor(
        const std::string& name, JSNativeObject<T> prototype):
    JSFunction(thunk) {
    setName(name);
    setProperty("prototype", prototype);
    prototype_ = JSGlobalValue(prototype);
}

template <class T>
void JSNativeConstructor<T>::setName(const std::string& name) {
    if (!name.empty()) {
        JSFunction::setName(name);
    } else {
#if __cpp_rtti || __GXX_RTTI
        size_t len;
        char *cname = abi::__cxa_demangle(typeid(T).name(), NULL, &len, NULL);
        JSFunction::setName(std::string(cname, len));
        free(cname);
#endif
    }
}

template <class T>
void JSNativeConstructor<T>::thunk(JSNIEnv* env, const JSNICallbackInfo info) {
    assert(env == JSValue::env);
    JSObject self = JSNIGetThisOfCallback(env, info);
    JSObject prototype = self.prototype();

    /* JSNI has no support of FunctionCallbackInfo::IsConstructCall()
    if (prototype != prototype_) {
        // this is a normal function call
    }*/

    // construct call
    T* native = new(std::nothrow) T(self, info);
    if (native == nullptr) {
        // throw a JavaScript exception
        return;
    }
    JSNativeObject<T> result(native, 0, std::default_delete<T>());
    result.setPrototype(prototype);
    JSNISetReturnValue(env, info, result);
}

}
