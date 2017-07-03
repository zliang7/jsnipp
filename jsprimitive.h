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
#include <string>

#include <jsni.h>

#include "jsvalue.h"

namespace jsnipp {

class JSUndefined final : public JSValue {
public:
    JSUndefined(): JSValue(env_->NewUndefined()) {}

    JSUndefined(JsValue jsval): JSValue(jsval) {
        assert(is_undefined());
    }
};


class JSNull final : public JSValue {
public:
    JSNull(std::nullptr_t null = nullptr):
        JSValue(env_->NewNull()) {}

    JSNull(JsValue jsval): JSValue(jsval) {
        assert(is_null());
    }
};
#define null_js JSNull()


class JSBoolean final : public JSValue {
public:
    JSBoolean(bool value = false):
        JSValue(env_->NewBoolean(value)) {}

    JSBoolean(const JSValue& jsval):
        JSValue(from(jsval).jsval_) {}
    JSBoolean(JsValue jsval): JSValue(jsval) {
        assert(is_boolean());
    }

    operator bool() const {
        return env_->ToBool(jsval_);
    }

    static JSBoolean from(const JSValue& jsval);
};
// mimic the user-defined literal which doesn't support bool type
#define true_js JSBoolean(true)
#define false_js JSBoolean(false)


class JSNumber final : public JSValue {
public:
    JSNumber(double value = 0):
        JSValue(env_->NewNumber(value)) {}

    JSNumber(const JSValue& jsval):
        JSValue(from(jsval).jsval_) {}
    JSNumber(JsValue jsval): JSValue(jsval) {
        assert(is_number());
    }

    operator double() const  {
        return env_->ToDouble(jsval_);
    }

    static JSNumber from(const JSValue& jsval);
};
inline JSNumber operator "" _js(long double num) {
    return JSNumber(num);
}
inline JSNumber operator "" _js(unsigned long long num) {
    return JSNumber((double)num);
}


class JSString final : public JSValue {
public:
    JSString(const std::string& str = std::string()):
        JSValue(env_->NewStringFromUtf8(str.c_str(), str.length())){}
    JSString(const char* str): 
        JSValue(env_->NewStringFromUtf8(str, -1)){}

    JSString(const JSValue& jsval):
        JSValue(from(jsval).jsval_){}
    JSString(JsValue jsval): JSValue(jsval) {
        assert(is_string());
    }

    operator std::string() const {
        if (length() == 0)  return std::string();
        char *buf = env_->GetStringUtf8Chars(jsval_);
        std::string str(buf, length());
        env_->ReleaseStringUtf8Chars(jsval_, buf);
        return str;
    }

    size_t length() const {
        return env_->GetStringUtf8Length(jsval_);
    }

    static JSString from(const JSValue& jsval);
};
inline JSString operator "" _js(const char * str) {
    return JSString(str);
}
inline JSString operator "" _js(const char * str, std::size_t size) {
    return JSString(str);
}

}
