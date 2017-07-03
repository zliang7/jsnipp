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
#include <cxxabi.h>
#include <functional>
#include <typeinfo>

#include <jsni.h>

#include "jsfunction.h"
#include "jsobject.h"

namespace jsnipp {

template <class C>
using JSFactoryFunctionType = C* (*)(JSObject, JSArray);

template <class C, JSFactoryFunctionType<C> factory = nullptr>
class JSNativeConstructor : public JSFunction {
public:
    JSNativeConstructor(std::function<void(JSObject)> setup = nullptr):
        JSFunction([](JSNIEnv* env, const CallbackInfo info){
            assert(env == env_);
            JSObject self = env->GetThis(info);
#if __cpp_exceptions || __EXCEPTIONS
            try {
#endif
                C* native = (factory != nullptr)?
                        (*factory)(self, info) : new C(self, info);
                if (native)
                    env->SetReturnValue(info, adopt(native));
#if __cpp_exceptions || __EXCEPTIONS
            } catch (JSValue result) {
                env->SetReturnValue(info, result);
            }
#endif
        }) {
        // set function name
        char *name = abi::__cxa_demangle(typeid(C).name(), NULL, NULL, NULL);
        setName(name);
        free(name);

        JSNativeObject<C> cls(nullptr, false, setup);
        class_ = env_->NewGlobalValue(cls);
    }

    JSNativeConstructor(JSPropertyList list):
        JSNativeConstructor(std::bind([](JSPropertyList list, JSObject cls){
            for (auto& p: list)  cls.setProperty(p.first, p.second);
        }, list, std::placeholders::_1)) {}

    static JSNativeObject<C> adopt(C* native, bool unmanaged = false) {
        assert(class_);
        JSNativeObject<C> result(env_->GetGlobalValue(class_));
        result.reset(native, unmanaged);
        return result;
    }

private:
    static JsGlobalValue class_;
};

template <class C, JSFactoryFunctionType<C> factory>
JsGlobalValue JSNativeConstructor<C, factory>::class_ = nullptr;

}
