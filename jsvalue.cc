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

#include "jsvalue.h"
#include "jsprimitive.h"
#include "jsobject.h"
#include "jsarray.h"
#include "jsfunction.h"

namespace jsnipp {

JSEnvironment JSValue::env_ = nullptr;

JSValue JSValue::from(JsValue jsval) {
    if      (env_->IsUndefined(jsval))   return JSUndefined(jsval);
    else if (env_->IsNull(jsval))        return JSNull(jsval);
    else if (env_->IsBoolean(jsval))     return JSBoolean(jsval);
    else if (env_->IsNumber(jsval))      return JSNumber(jsval);
    else if (env_->IsString(jsval))      return JSString(jsval);
    else if (env_->IsFunction(jsval))    return JSFunction(jsval);
    else if (env_->IsArray(jsval))       return JSArray(jsval);
//  else if (env_->IsTypedArray(jsval))  return JSTypedArray(jsval);
    else if (env_->IsObject(jsval))      return JSObject(jsval);
    assert(env_->IsEmpty(jsval));
    return JSUndefined();
}


JSValue JSValue::from(void) {
    return JSUndefined();
}
JSValue JSValue::from(std::nullptr_t null) {
    return JSNull();
}
JSValue JSValue::from(bool boolean) {
    return JSBoolean(boolean);
}
JSValue JSValue::from(double number) {
    return JSNumber(number);
}
JSValue JSValue::from(const std::string& string) {
    return JSString(string);
}
JSValue JSValue::from(const char* cstring) {
    return JSString(cstring);
}
JSValue JSValue::from(NativeFunctionCallback callback) {
    return JSFunction(env_->NewFunction(callback));
}
template<typename T>
JSValue JSValue::from(const std::vector<T>& array) {
    return JSArray(array);
}
template<typename T>
JSValue JSValue::from(const std::map<std::string, T>& object) {
    return JSObject(object);
}

}
