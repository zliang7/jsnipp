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
#include <functional>

#include <jsni.h>

#include "jsfunction.h"

namespace jsnipp {

template <class T>
class JSNativeConstructor : public JSFunction {
public:
    JSNativeConstructor(std::function<std::string(JSObject)> setup);
    JSNativeConstructor(JSPropertyList list);

    static JSNativeObject<T> adopt(T* native) {
        assert(class_);
        JSNativeObject<T> result(env_->GetGlobalValue(class_));
        result.reset(native);
        return result;
    }

private:
    static void thunk(JSNIEnv* env, const CallbackInfo info);
    static JsGlobalValue class_;
};

}


//////////////////////////////////////////////////////////////////////////////

#if __cpp_rtti || __GXX_RTTI
#include <cxxabi.h>
#include <typeinfo>
#endif

namespace jsnipp {

namespace {

inline std::string setup_list(JSObject cls, JSPropertyList list) {
    std::string name;
    for (auto& p: list) {
        if (p.first != "name")
            cls.setProperty(p.first, p.second);
        else
            name = JSString(p.second);
    }
    return name;
}

}


template <class T>
JsGlobalValue JSNativeConstructor<T>::class_ = nullptr;

template <class T>
JSNativeConstructor<T>::JSNativeConstructor(
        std::function<std::string(JSObject)> setup):
    JSFunction(thunk) {
    JSNativeObject<T> cls(nullptr);
    std::string name;
    if (setup)
        name = setup(cls);
#if __cpp_rtti || __GXX_RTTI
    if (name.empty()) {
        size_t len;
        char *cname = abi::__cxa_demangle(typeid(T).name(), NULL, &len, NULL);
        name.append(cname, len);
        free(cname);
    }
#endif
    if (!name.empty())
        setName(name);
    class_ = env_->NewGlobalValue(cls);
}

template <class T>
JSNativeConstructor<T>::JSNativeConstructor(JSPropertyList list):
    JSNativeConstructor<T>(
            std::bind(setup_list, std::placeholders::_1, list)) {}

template <class T>
void JSNativeConstructor<T>::thunk(JSNIEnv* env, const CallbackInfo info) {
    assert(env == env_);
    JSObject self = env->GetThis(info);
#if __cpp_exceptions || __EXCEPTIONS
    try {
#endif
        T* native = new T(self, info);
        if (native)
            env->SetReturnValue(info, adopt(native));
#if __cpp_exceptions || __EXCEPTIONS
    } catch (JSValue result) {
        env->SetReturnValue(info, result);
    }
#endif
}

}
