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
    JSNativeConstructor(std::function<void(JSObject&)> setup,
                        const std::string& name = std::string());
    JSNativeConstructor(JSPropertyList list,
                        const std::string& name = std::string());

    static JSNativeObject<T> adopt(T* native) {
/*      assert(class_);
        JSNativeObject<T> result(env_->GetGlobalValue(class_));
        result.reset(native);
        return result;*/
        return JSNativeObject<T>(native, setup_);
    }

private:
    static void thunk(JSNIEnv* env, const CallbackInfo info);
    static std::function<void(JSObject&)> setup_;
    //static JsGlobalValue class_;
};

}


//////////////////////////////////////////////////////////////////////////////

#if __cpp_rtti || __GXX_RTTI
#include <cxxabi.h>
#include <typeinfo>
#endif

namespace jsnipp {

namespace {

inline void setup_list(JSObject& cls, JSPropertyList list) {
    for (auto& p: list)
        cls.setProperty(p.first, p.second);
}

}


template <class T>
//JsGlobalValue JSNativeConstructor<T>::class_ = nullptr;
std::function<void(JSObject&)> JSNativeConstructor<T>::setup_ = nullptr;

template <class T>
JSNativeConstructor<T>::JSNativeConstructor(
        std::function<void(JSObject&)> setup, const std::string& name):
    JSFunction(thunk) {
    /*JSNativeObject<T> cls(nullptr);
    std::string name;
    if (setup)
        name = setup(cls);*/
    if (!name.empty()) {
        setName(name);
    } else {
#if __cpp_rtti || __GXX_RTTI
        size_t len;
        char *cname = abi::__cxa_demangle(typeid(T).name(), NULL, &len, NULL);
        setName(std::string(cname, len));
        free(cname);
#endif
    }
    //class_ = env_->NewGlobalValue(cls);
    assert(!setup_);
    setup_ = setup;
}

template <class T>
JSNativeConstructor<T>::JSNativeConstructor(JSPropertyList list,
                                            const std::string& name):
    JSNativeConstructor<T>(
            std::bind(setup_list, std::placeholders::_1, list), name) {}

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
