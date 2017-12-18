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

#include "jsobject.h"

namespace jsni {

class JSArray final : public JSObject {
public:
    JSArray(JSValueRef jsval): JSObject(jsval, true) {
        assert(check(*this));
    }

    explicit JSArray(size_t length = 0):
        JSObject(JSNINewArray(env, length), true) {}
    typedef std::initializer_list<JSValue> initializer_list;
    JSArray(initializer_list list): JSArray(list.size()) {
        for (auto p = list.begin(); p < list.end(); ++p)
            setElement(p - list.begin(), *p);
    }
    template <class T, typename = typename std::enable_if<
            std::is_constructible<JSValue, typename T::value_type>::value>::type>
    JSArray(const T& seq): JSArray(std::distance(seq.begin(), seq.end())) {
        size_t index = 0;
        for (auto it = seq.begin(); it != seq.end(); ++it)
            setElement(index++, *it);
    }

    template <typename... Ts>
    JSArray(size_t size, Ts&&... args):
        JSArray(std::max(size, sizeof...(Ts))) {
        reduce_args(sizeof...(Ts), std::forward<Ts>(args)...);
    }
/*  template <typename... Ts>
    JSArray(std::tuple<Ts...> tuple): JSArray(sizeof...(Ts)) {
        TupleHelper<decltype(tuple), sizeof...(Ts)>::reduce(*this, tuple);
    }*/

    JSArray(const JSNICallbackInfo info):
        JSArray(JSNIGetArgsLengthOfCallback(env, info)) {
        size_t len = JSNIGetArgsLengthOfCallback(env, info);
        for (size_t i = 0; i < len; ++i)
            setElement(i, JSNIGetArgOfCallback(env, info, i));
    }

    size_t length() const {
        return JSNIGetArrayLength(env, jsval_);
    }
    // TODO: support more methods of Array

    template <typename T>
    T getElement(size_t index, JSTypeID<T> = JSTypeID<T>()) const {
        return getElement(index).to<T>();
    }
    JSValue getElement(size_t index) const {
        return JSNIGetArrayElement(env, jsval_, index);
    }
    void setElement(size_t index, JSValue value) {
        JSNISetArrayElement(env, jsval_, index, value);
    }
    JSValue operator [](int index) const {
        return getElement(index);
    }

/*  class Accessor final : public JSValue {
    public:
        Accessor& operator=(JSValue value) {
            array_.setElement(index_, value);
            return *this;
        }
        Accessor& operator=(const Accessor& value) {
            return operator=(JSValue(value));
        }
    private:
        Accessor(JSArray& array, size_t index):
            JSValue(array.getElement(index)),
            array_(array), index_(index){}

        JSArray& array_;
        size_t index_;
        friend class JSArray;
    };
    Accessor operator [](int index) {
        return Accessor(*this, index);
    }*/

    static bool check(JSValueRef jsval) {
        return JSNIIsArray(env, jsval);
    }
    static JSArray from(JSValue jsval);

private:
    void reduce_args(size_t length) {}
    template <typename T, typename... Ts>
    void reduce_args(size_t length, T first, Ts&&... args) {
        size_t index = length - sizeof...(Ts) - 1;
        setElement(index, first);
        reduce_args(length, std::forward<Ts>(args)...);
    }
};

#if 0
namespace {
template<class Tuple, std::size_t N>
struct TupleHelper {
    static void reduce(JSArray array, const Tuple& tuple) {
        TupleHelper<Tuple, N-1>::reduce(array, tuple);
        array.setElement(N-1, std::get<N-1>(tuple));
    }
};
template<class Tuple>
struct TupleHelper<Tuple, 1> {
    static void reduce(JSArray array, const Tuple& tuple) {
         array.setElement(0, std::get<0>(tuple));
    }
};
}
#endif

inline JSArray JSArray::from(JSValue jsval) {
    if (jsval.is(Array))  return jsval.as(Array);
/*  if (jsval.is(Number))
        return JSArray((size_t)JSNumber(jsval));*/
    return JSArray(1, jsval);
}

}
