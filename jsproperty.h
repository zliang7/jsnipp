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

namespace jsnipp {

class JSFunction;

class JSPropertyDescriptor : public JSObject {
public:
    bool configurable() const {
        return JSBoolean(getProperty("configurable"));
    }
    bool enumerable() const {
        return JSBoolean(getProperty("enumerable"));
    }
    void set_configurable(bool val) {
        setProperty("configurable", JSBoolean(val));
    }
    void set_enumerable(bool val) {
        setProperty("enumerable", JSBoolean(val));
    }

protected:
    JSPropertyDescriptor(bool configurable, bool enumerable): JSObject() {
        if (configurable) set_configurable(true);
        if (enumerable)   set_enumerable(true);
    }
};

class JSPropertyData final : public JSPropertyDescriptor {
public:
    JSPropertyData(JSValue value, bool writable = false,
                   bool configurable = false, bool enumerable = false);
};

class JSPropertyAccessor final : public JSPropertyDescriptor {
public:
    JSPropertyAccessor(JSFunction getter,
                       bool configurable = false, bool enumerable = false);
    JSPropertyAccessor(JSFunction getter, JSFunction setter,
                       bool configurable = false, bool enumerable = false);
};

}


#include "jsfunction.h"

namespace jsnipp {

inline JSPropertyData::JSPropertyData(
        JSValue value, bool writable, bool configurable, bool enumerable):
    JSPropertyDescriptor(configurable, enumerable) {
    setProperty("value", value);
    if (writable) setProperty("writable", true_js);
}

inline JSPropertyAccessor::JSPropertyAccessor(
        JSFunction getter, bool configurable, bool enumerable):
    JSPropertyDescriptor(configurable, enumerable) {
    setProperty("get", getter);
}
inline JSPropertyAccessor::JSPropertyAccessor(
        JSFunction getter, JSFunction setter, bool configurable, bool enumerable):
    JSPropertyAccessor(getter, configurable, enumerable) {
    setProperty("set", setter);
}

}
