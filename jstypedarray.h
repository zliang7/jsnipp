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
#include <type_traits>

#include "jsobject.h"

namespace jsni {

template<typename T, bool clamped = false>
class JSTypedArray final : public JSObject {
public:
    static_assert(std::is_arithmetic<T>::value || std::is_void<T>::value, "");
    static_assert(std::is_same<T, uint8_t>::value || !clamped, "");

    JSTypedArray(JSValueRef jsval): JSObject(jsval, true) {
        assert(check(*this));
    }

    JSTypedArray(): JSTypedArray(nullptr, 0) {}
    JSTypedArray(T* data, size_t length):  // TODO: support deleter
        JSObject(JSNINewTypedArray(env, type(), data, length), true) {}

    size_t length() const {
        return JSNIGetTypedArrayLength(env, jsval_);
    }
    size_t byteLength() const {
        using U = typename
            std::conditional<std::is_same<T, void>::value, char, T>::type;
        return length() * sizeof(U);
    }
    T* buffer() const {
        return reinterpret_cast<T*>(JSNIGetTypedArrayData(env, jsval_));
    }

    constexpr static JsTypedArrayType type() {
        return std::is_same<T, int8_t>::value   ? JsArrayTypeInt8 : (
               std::is_same<T, uint8_t>::value  ? (clamped ?
                        JsArrayTypeUint8Clamped : JsArrayTypeUint8): (
               std::is_same<T, int16_t>::value  ? JsArrayTypeInt16 : (
               std::is_same<T, uint16_t>::value ? JsArrayTypeUint16 : (
               std::is_same<T, int32_t>::value  ? JsArrayTypeInt32 : (
               std::is_same<T, uint32_t>::value ? JsArrayTypeUint32 : (
               std::is_same<T, float>::value    ? JsArrayTypeFloat32 : (
               std::is_same<T, double>::value   ? JsArrayTypeFloat64 : (
               JsArrayTypeNone))))))));
    }

    static bool check(JSValueRef value) {
        return JSNIIsTypedArray(env, value) && (
                   std::is_void<T>::value ||
                   JSNIGetTypedArrayType(env, value) == type()
               );
    }
};
/*
template<typename T, bool clamped>
constexpr JsTypedArrayType JSTypedArray<T, clamped>::type() {
    if (std::is_same<T, int8_t>::value)
        return JsArrayTypeInt8;
    else if (std::is_same<T, uint8_t>::value)
        return clamped ? JsArrayTypeUint8Clamped : JsArrayTypeUint8;
    else if  (std::is_same<T, int16_t>::value)
        return JsArrayTypeInt16;
    else if  (std::is_same<T, uint16_t>::value)
        return JsArrayTypeUint16;
    else if  (std::is_same<T, int32_t>::value)
        return JsArrayTypeInt32;
    else if  (std::is_same<T, uint32_t>::value)
        return JsArrayTypeUint32;
    else if  (std::is_same<T, float>::value)
        return JsArrayTypeFloat32;
    else if  (std::is_same<T, double>::value)
        return JsArrayTypeFloat64;
    else
        return JsArrayTypeNone;
}*/

}
