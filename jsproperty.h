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
#include "jsfunction.h"

namespace jsni {

class JSPropertyDescriptor {
public:
    // import from a JavaScript descriptor
    explicit JSPropertyDescriptor(JSObject descriptor);

    // construct form JSNI raw type
    JSPropertyDescriptor(const JSNIAccessorPropertyDescriptor& desc):
        accessor_(desc), getter_(desc.getter), setter_(desc.setter) {}
    JSPropertyDescriptor(const JSNIDataPropertyDescriptor& desc):
        data_(desc), getter_(nullptr), setter_(nullptr) {}
    JSPropertyDescriptor(JSNIPropertyAttributes attributes):
        getter_(nullptr), setter_(nullptr) {
        value_ = nullptr;
        attrib_ = attributes;
    }

    JSPropertyDescriptor(JSValue value, bool writable = true,
                         bool enumerable = true, bool configurable = false) {
        set_configurable(configurable);
        set_enumerable(enumerable);
        set_data(value, writable);
    }
    JSPropertyDescriptor(JSFunction getter, JSFunction setter = nullptr,
                         bool enumerable = true, bool configurable = false) {
        set_configurable(configurable);
        set_enumerable(enumerable);
        set_accessor(getter, setter);
    }

    operator JSObject() const;
    operator JSNIPropertyDescriptor() const;

    bool configurable() const {
        return !(attrib_ & JSNIDontDelete);
    }
    void set_configurable(bool v) {
        attrib_ = (JSNIPropertyAttributes)
            (v ? attrib_ & ~JSNIDontDelete : attrib_ | JSNIDontDelete);
    }
    bool enumerable() const {
        return !(attrib_ & JSNIDontEnum);
    }
    void set_enumerable(bool v) {
        attrib_ = (JSNIPropertyAttributes)
            (v ? attrib_ & ~JSNIDontEnum : attrib_ | JSNIDontEnum);
    }

    JSValue value() const {
        return value_;
    }
    bool writable() const {
        return !(attrib_ & JSNIReadOnly);
    }
    void set_data(JSValue value, bool writable = false) {
        getter_ = setter_ = nullptr;
        value_ = value;
        attrib_ = (JSNIPropertyAttributes)
            (writable ? attrib_ & ~JSNIReadOnly : attrib_ | JSNIReadOnly);
    }
    bool is_data_descriptor() const {
        return !is_accessor_descriptor();
    }

    JSFunction getter() const {
        return getter_;
    }
    JSFunction setter() const {
        return setter_;
    }
    void set_accessor(JSFunction getter, JSFunction setter = nullptr) {
        accessor_.getter = getter_ = getter;
        accessor_.setter = setter_ = setter;
    }
    bool is_accessor_descriptor(bool native = false) const {
        if (!getter_ && !setter_)  return false;
        return !native ||
               !((getter_ && !accessor_.getter) ||
                 (setter_ && !accessor_.setter));
    }

private:
    static constexpr size_t padding_size =
        offsetof(JSNIAccessorPropertyDescriptor, attributes) -
        offsetof(JSNIDataPropertyDescriptor, attributes);
    union {
        struct {
            char padding1[padding_size];
            JSNIDataPropertyDescriptor data_;
        };
        JSNIAccessorPropertyDescriptor accessor_;
    };
    JSValueRef& value_ = data_.value;
    JSNIPropertyAttributes& attrib_ = data_.attributes;
    JSFunction getter_;
    JSFunction setter_;
};

}


namespace jsni {

inline JSPropertyDescriptor::JSPropertyDescriptor(JSObject jsobj) {
    JSFunction getter, setter;
    if (jsobj["get"].is(Function))
        getter = JSFunction(jsobj["get"]);
    if (jsobj["set"].is(Function))
        getter = JSFunction(jsobj["set"]);
    if (getter || setter) {
        set_accessor(getter, setter);
    } else {
        bool writable = jsobj["writable"].to(Boolean);
        set_data(jsobj["value"], writable);
    }
    set_configurable(jsobj["configurable"].to(Boolean));
    set_enumerable(jsobj["enumerable"].to(Boolean));
}

inline JSPropertyDescriptor::operator JSObject() const {
    JSObject desc {
        {"configurable", configurable() },
        {"enumerable", enumerable() }
    };
    if (is_data_descriptor()) {
        desc["writable"] = writable();
        desc["value"] = value();
    } else {
        desc["get"] = getter_;
        desc["set"] = setter_;
    }
    return desc;
}

inline JSPropertyDescriptor::operator JSNIPropertyDescriptor() const {
    JSNIPropertyDescriptor desc = { nullptr,  nullptr};
    if (is_data_descriptor()) {
        desc.data_attributes =
            const_cast<JSNIDataPropertyDescriptor*>(&data_);
    } else if (is_accessor_descriptor(true)) {
        desc.accessor_attributes =
            const_cast<JSNIAccessorPropertyDescriptor*>(&accessor_);
    }
    return desc;
}

}
