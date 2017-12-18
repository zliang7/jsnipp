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

#include "jsprimitive.h"
#include "jsobject.h"
#include "jsarray.h"
#include "jsfunction.h"

namespace jsni {

/* JSValue type conversion matrix

           | boolean | number |     string     |    array     |   object
-----------+---------+--------+----------------+--------------+------------
 undefined |  false  |  NaN   |  'undefined'   | [undefined]  |     {}
   null    |  false  |   0    |     'null'     |    [null]    |     {}
  boolean  |    =    |  0/1   | 'true'/'false' | [true/false] | Boolean(x)
  number   |   (1)   |   =    |   toString()   |    [num]     | Number(x)
  string   |   (2)   | parse  |    =           |    [str]     | String(x)
  array    |  true   |  (3)   |   toString()   |      =       |     =
  object   |  true   |  NaN   |   toString()   |    [obj]     |     =
 function  |  true   |  NaN   |   toString()   |    [func]    |     =

1. num == 0 || num == NaN
2. str.length() > 0
3. len ? (len == 1 ? parse(arr[0]) : NaN) : 0
*/

inline JSBoolean JSBoolean::from(JSValue jsval) {
    if (jsval.is(Boolean))  return jsval.as(Boolean);

    bool val = false;
    if (jsval.is(Number)) {
        double d = jsval.as(Number);
        val = d != 0.0 && !isnan(d);
    } else if (jsval.is(String)) {
        val = jsval.as(String).length() > 0;
    } else {
        val = !jsval.is(Null) && !jsval.is(Undefined);
    }
    return JSBoolean(val);
}

inline JSNumber JSNumber::from(JSValue jsval) {
    if (jsval.is(Number))  return jsval.as(Number);

    double num = 0.0;
    if (jsval.is(Boolean)) {
        num = jsval.as(Boolean) ? 1.0 : 0.0;
    } else if (jsval.is(String)) {
        std::string str = jsval.as(String);
        if (str.length() > 0) {
            char* end;
            num = strtod(str.c_str(), &end);
            if (*end)
                num = NAN;
            else if (std::abs(num) == HUGE_VAL)
                num = copysign(INFINITY, num);
        }
    } else if (jsval.is(Array)) {
        auto array = jsval.as(Array);
        if (array.length() == 1)
            return from(array[0]);
        if (array.length() > 0)
            num = NAN;
    } else if (!jsval.is(Null)) {
        num = NAN;
    }
    return JSNumber(num);
}

inline JSString JSString::from(JSValue jsval) {
    if (jsval.is(String))  return jsval.as(String);

    std::string str;
    if (jsval.is(Null)) {
        str = "null";
    } else if (jsval.is(Undefined)) {
        str = "undefined";
    } else if (jsval.is(Boolean)) {
        str = jsval.as(Boolean) ? "true" : "false";
    } else if (jsval.is(Number)) {
        str = std::to_string((double)jsval.as(Number));
    } else {
        return jsval.to(Object).callMethod("toString").as(String);
    }
    return JSString(str);
}

}
