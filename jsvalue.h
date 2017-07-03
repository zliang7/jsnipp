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
#include <map>
#include <string>
#include <vector>

#include <jsni.h>

namespace jsnipp {

class JSValue;
class JSEnvironment final {
public:
    const JSNIEnv* operator->() const {
        return env_;
    }
    operator JSNIEnv*() const {
        return env_;
    }

    int version() const {
        return env_->GetVersion();
    }

private:
    JSEnvironment(const JSNIEnv* env): env_(env) {}
    const JSNIEnv* env_ = nullptr;

    friend class JSValue;
};


class JSValue {
public:
    static void setup(JSNIEnv* env) {
        assert(env_ == nullptr && env != nullptr);
        env_ = env;
    }

    static JSValue from(void);
    static JSValue from(std::nullptr_t null);
    static JSValue from(bool boolean);
    static JSValue from(double number);
    static JSValue from(const std::string& string);
    static JSValue from(const char* cstring);
    static JSValue from(NativeFunctionCallback func);
    template<typename T>
    static JSValue from(const std::map<std::string, T>& object);
    template<typename T>
    static JSValue from(const std::vector<T>& array);
    static JSValue from(JsValue jsval);
    static JSValue from(const JSValue& jsval) { return jsval; }

    bool is_undefined() const  { return env_->IsUndefined(jsval_); }
    bool is_null() const       { return env_->IsNull(jsval_); }
    bool is_boolean() const    { return env_->IsBoolean(jsval_); }
    bool is_number() const     { return env_->IsNumber(jsval_); }
    bool is_string() const     { return env_->IsString(jsval_); }
    bool is_object() const     { return env_->IsObject(jsval_); }
    bool is_function() const   { return env_->IsFunction(jsval_); }
    bool is_array() const      { return env_->IsArray(jsval_); }
    bool is_typedarray() const { return env_->IsTypedArray(jsval_); }
    bool is_valid() const      { return !env_->IsEmpty(jsval_); }

    bool operator ==(const JSValue& that) const {
        return false; // TODO
    }
    operator JsValue() const {
        return jsval_;
    }

    // clang will complain the class is not a POD type without:
    JSValue() = delete;

protected:
    JSValue(JsValue jsval): jsval_(jsval) {}
    JsValue jsval_;

    static JSEnvironment env_;
    friend class JSGlobalValue;
    friend class JSScope;
    friend class JSEscapableScope;
};


class JSGlobalValue final {
public:
    JSGlobalValue(): jsgval_(nullptr) {}
    JSGlobalValue(JsValue jsval):
        jsgval_(env()->NewGlobalValue(jsval)) {}
    JSGlobalValue(JSGlobalValue&& gv): jsgval_(gv.jsgval_) {
        gv.jsgval_ = nullptr;
    }
    JSGlobalValue(const JSGlobalValue&) = delete;
    ~JSGlobalValue() {
        if (jsgval_ != nullptr)
            env()->DeleteGlobalValue(jsgval_);
    }

    operator JSValue() const {
        assert(jsgval_);
        return JSValue::from(env()->GetGlobalValue(jsgval_));
    }
    operator JsGlobalValue() const {
        return jsgval_;
    }
    operator bool() const {
        return jsgval_ != nullptr;
    }
    bool operator ==(const JSGlobalValue& that) const {
        // FIXME: persistent handle should not be used to identify an object.
        return jsgval_ == that.jsgval_;
    }

    void makeWeak(JSNIGCCallback callback, void* data = nullptr) {
        assert(jsgval_ != nullptr && callback != nullptr);
        env()->SetWeakGCCallback(jsgval_, data, callback);
    }

private:
    JsGlobalValue jsgval_;
    static JSNIEnv* env() {
        return JSValue::env_;
    }
};


class JSEscapableScope {
public:
    JSEscapableScope() {
        JSValue::env_->PushLocalScope();
    }
    JSValue escape(JSValue jsval) {
        return JSValue::env_->PopLocalScope(jsval);
    }
};
class JSScope final : JSEscapableScope {
public:
    ~JSScope() {
        JSValue::env_->PopLocalScope(nullptr);
    }
    JSValue escape(JSValue jsval) = delete;
};

}
