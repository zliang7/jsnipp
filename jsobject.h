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
#include <string>

#include <jsni.h>

#include "jsvalue.h"
#include "jsprimitive.h"

namespace jsnipp {

typedef std::initializer_list<std::pair<std::string, JSValue>> JSPropertyList;

class JSPropertyDescriptor;

class JSObject : public JSValue {
public:
    JSObject(): JSValue(env_->NewObject()) {}
    JSObject(std::nullptr_t): JSValue(env_->NewNull()) {}
    JSObject(JSPropertyList list): JSObject() {
        for (auto& p: list)
            setProperty(p.first.c_str(), p.second);
    }

    JSObject(const JSValue& jsval);
    JSObject(JsValue jsval): JSValue(jsval) {
        assert(is_object());
    }

    JSObject prototype() const {
        return JSObject(env_->GetPrototype(jsval_));
    }

    JSString toString() const {
        JSValue jsfunc = getProperty("toString");
        return env_->CallFunction(jsfunc, jsval_, 0, NULL);
    }

    JSValue getProperty(const std::string& key) const {
        return from(env_->GetProperty(jsval_, key.c_str()));
    }
    bool setProperty(const std::string& key, JSValue jsval) {
        return env_->SetProperty(jsval_, key.c_str(), jsval);
    }
    bool hasProperty(const std::string& key) const {
        return env_->HasProperty(jsval_, key.c_str());
    }
    bool deleteProperty(const std::string& key) {
        return env_->DeleteProperty(jsval_, key.c_str());
    }
    void defineProperty(const std::string& name, JSPropertyDescriptor descriptor);

    class Accessor final {
    public:
        operator JSValue() const {
            return obj_.getProperty(key_);
        }
        Accessor& operator=(JSValue value) {
            obj_.setProperty(key_, value);
            return *this;
        }
        Accessor& operator=(const Accessor& ref) {
            obj_.setProperty(key_, ref);
            return *this;
        }

    private:
        Accessor(const Accessor&) = default;
        Accessor(JSObject& obj, const std::string& key):
            obj_(obj), key_(key){}

        const std::string& key_;
        JSObject& obj_;
        friend class JSObject;
    };
    JSValue operator [](const std::string& key) const {
        return getProperty(key);
    }
    Accessor operator [](const std::string& key) {
        return Accessor(*this, key);
    }

protected:
    JSObject(int cnt):
        JSValue(env_->NewObjectWithHiddenField(cnt)) {}
};


template<class T>
class JSNativeObject : public JSObject {
public:
    typedef void (*Deleter)(T*);
    JSNativeObject(T* native, unsigned int count = 0,
                   Deleter deleter = JSNativeObject<T>::defaultDeleter);
    JSNativeObject(std::nullptr_t): JSNativeObject(nullptr, 0) {}

    JSNativeObject(T* native, std::function<void(T&, JSObject)> setup):
        JSNativeObject(native) {
        setup(*this, jsval_);
    }

    JSNativeObject(T* native, JSPropertyList list):
        JSNativeObject(native) {
        for (auto& p: list)
            setProperty(p.first, p.second);
    }

    JSNativeObject(JsValue jsval);

    T* native() const {
        int index = env_->HiddenFieldCount(jsval_) - 1;
        return reinterpret_cast<T*>(getPrivate(index));
    }
    void reset(T* native);

protected:
    void* getPrivate(int index) const {
        assert(index < env_->HiddenFieldCount(jsval_) && index >= 0);
        return env_->GetHiddenField(jsval_, index);
    }
    void setPrivate(int index, void* ptr) {
        assert(index < env_->HiddenFieldCount(jsval_) && index >= 0);
        return env_->SetHiddenField(jsval_, index, ptr);
    }
    static void defaultDeleter(T* obj) {
        delete obj;
    }
};

}


#include "jsproperty.h"

namespace jsnipp {

inline void JSObject::defineProperty(const std::string& name,
                                     JSPropertyDescriptor descriptor) {
    JSObject object(JSObject().prototype()["constructor"]);
    JSFunction define(object["defineProperty"]);
    define.call(object, *this, name, descriptor);
}


template<class T>
JSNativeObject<T>::JSNativeObject(T* native, unsigned int count,
                                  Deleter deleter):
    JSObject(count + 3) {
    setPrivate(count + 2, native);
#if (__cpp_rtti || defined(__GXX_RTTI)) && !defined(NDEBUG)
    auto hash = static_cast<uintptr_t>(typeid(T).hash_code());
    setPrivate(count, reinterpret_cast<void*>(hash));
#endif

    if (deleter == nullptr)  return;
    setPrivate(count + 1, reinterpret_cast<void*>(deleter));
    env_->SetGCCallback(jsval_, jsval_, [](JSNIEnv*, void* data){
        JSNativeObject<T> self(reinterpret_cast<JsValue>(data));
        self.reset(nullptr);
    });
}

template<class T>
JSNativeObject<T>::JSNativeObject(JsValue jsval): JSObject(jsval) {
    assert(is_object());
    int index = env_->HiddenFieldCount(jsval_) - 3;
    assert(index >= 0);
#if (__cpp_rtti || defined(__GXX_RTTI)) && !defined(NDEBUG)
    auto hash = reinterpret_cast<uintptr_t>(getPrivate(index));
    assert(typeid(T).hash_code() == static_cast<size_t>(hash));
#endif
}

template<class T>
void JSNativeObject<T>::reset(T* native) {
    T* old = this->native();
    if (native == old)  return;
    int index = env_->HiddenFieldCount(jsval_) - 1;
    setPrivate(index, native);

    auto deleter = reinterpret_cast<Deleter>(getPrivate(index - 1));
    if (deleter != nullptr)
        deleter(old);
}

}
