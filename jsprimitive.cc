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

#include <cmath>

#include "jsprimitive.h"
#include "jsobject.h"

namespace jsnipp {

JSBoolean JSBoolean::from(const JSValue& jsval) {
    if (jsval.is_boolean())
        return static_cast<JsValue>(jsval);

    bool value = false;
    if (jsval.is_number()) {
        double num = env_->ToDouble(jsval);
        value = num != 0 && !isnan(num);
    } else if (jsval.is_string()) {
        value = env_->GetStringUtf8Length(jsval) > 0;
    } else if (jsval.is_object()) {
        value = true;
    }
    return JSBoolean(value);
}

JSNumber JSNumber::from(const JSValue& jsval) {
    if (jsval.is_number())
        return static_cast<JsValue>(jsval);
    if (jsval.is_array() && env_->GetArrayLength(jsval) == 1)
        return from(JSValue::from(env_->GetArrayElement(jsval, 0)));

    double num = 0;
    if (jsval.is_string()) {
        if (env_->GetStringUtf8Length(jsval) > 0) {
            char *str = env_->GetStringUtf8Chars(jsval), *end = str;
            num = strtod(str, &end);
            if (end == str || *end != '\0')
                num = NAN;
            env_->ReleaseStringUtf8Chars(jsval, str);
        }
    } else if (jsval.is_boolean()) {
        num = env_->ToBool(jsval) ? 1 : 0;
    } else if (!jsval.is_null()) {
        num = NAN;
    }
    return JSNumber(num);
}

JSString JSString::from(const JSValue& jsval) {
    if (jsval.is_string())
        return static_cast<JsValue>(jsval);
    if (jsval.is_object())
        return JSObject(jsval).toString();

    std::string str;
    if (jsval.is_number()) {
        str = std::to_string(env_->ToDouble(jsval));
    } else if (jsval.is_boolean()) {
        str = env_->ToBool(jsval) ? "true" : "false";
    } else if (jsval.is_null()) {
        str = "null";
    } else if (jsval.is_undefined()) {
        str = "undefined";
    } else {
        assert(false); // any type should be string convertible.
    }
    return JSString(str);
}

}
