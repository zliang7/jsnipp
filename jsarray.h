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
#include <vector>

#include <jsni.h>

#include "jsobject.h"

namespace jsnipp {

class JSArray final : public JSObject {
public:
    JSArray(): JSObject(env_->NewArray(0)) {}

    template <typename... Ts>
    JSArray(size_t size, Ts... args):
        JSObject(env_->NewArray(std::max(size, sizeof...(Ts)))) {
        reduce_args(sizeof...(Ts), args...);
    }
/*  template <typename... Ts>
    JSArray(std::tuple<Ts...> tuple): JSArray(sizeof...(Ts)) {
        TupleHelper<decltype(tuple), sizeof...(Ts)>::reduce(*this, tuple);
    }*/

    template <typename T = JSValue>
    JSArray(std::initializer_list<T> init): JSArray(init.size()) {
        size_t i = 0;
        for (auto& e: init)
            env_->SetArrayElement(jsval_, i++, from(e));
    }

    JSArray(const JSValue& jsval): JSArray(1) {
        if (jsval.is_array())  return;

        if (jsval.is_number()) {
            jsval_ = env_->NewArray(env_->ToDouble(jsval));
        } else {
            jsval_ = env_->NewArray(1);
            env_->SetArrayElement(jsval_, 0, jsval);
        }
    }
    JSArray(JsValue jsval): JSObject(jsval) {
        assert(is_array());
    }
    JSArray(CallbackInfo info): JSArray(env_->GetArgsLength(info)) {
        size_t len = env_->GetArgsLength(info);
        for (size_t i = 0; i < len; ++i)
            setElement(i, from(env_->GetArg(info, i)));
    }

    size_t length() const {
        return env_->GetArrayLength(jsval_);
    }

    JSValue getElement(size_t index) const {
        return from(env_->GetArrayElement(jsval_, index));
    }
    void setElement(size_t index, JSValue value) {
        env_->SetArrayElement(jsval_, index, value);
    }
    JSValue operator [](int index) const {
        return getElement(index);
    }

private:
    void reduce_args(size_t length) {}
    template <typename T, typename... Ts>
    void reduce_args(size_t length, T first, Ts... args) {
        size_t index = length - sizeof...(Ts) - 1;
        setElement(index, JSValue::from(first));
        reduce_args(length, args...);
    }
};

#if 0
namespace {
template<class Tuple, std::size_t N>
struct TupleHelper {
    static void reduce(JSArray array, const Tuple& tuple) {
        TupleHelper<Tuple, N-1>::reduce(array, tuple);
        array.setElement(N-1, JSValue::from(std::get<N-1>(tuple)));
    }
};
template<class Tuple>
struct TupleHelper<Tuple, 1> {
    static void reduce(JSArray array, const Tuple& tuple) {
         array.setElement(0, JSValue::from(std::get<0>(tuple)));
    }
};
}
#endif
}
