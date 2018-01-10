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

#include <cerrno>
#include <cmath>
#include <string>

#include "jsvalue.h"

namespace jsni {

class JSUndefined final : public JSValue {
public:
    JSUndefined(JSValueRef jsval): JSValue(jsval) {
        if (!check(jsval_))  jsval_ = JSUndefined();
    }

    JSUndefined(): JSValue(JSNINewUndefined(env)) {}

    static bool check(JSValueRef jsval) {
        return JSNIIsUndefined(env, jsval);
    }
};


class JSNull final : public JSValue {
public:
    JSNull(JSValueRef jsval): JSValue(jsval) {
        if (!check(jsval_))  jsval_ = JSNull();
    }

    JSNull(std::nullptr_t = nullptr):
        JSValue(JSNINewNull(env)) {}

    static bool check(JSValueRef jsval) {
        return JSNIIsNull(env, jsval);
    }
};
#define null_js JSNull()


class JSBoolean final : public JSValue {
public:
    JSBoolean(JSValueRef jsval);
    explicit JSBoolean(JSValueRef jsval, JSBoolean defval):
        JSValue(jsval) {
        if (!check(jsval_))  jsval_ = defval;
    }

    JSBoolean(bool value = false):
        JSValue(JSNINewBoolean(env, value)) {}

    // conversion
    operator bool() const {
        return JSNIToCBool(env, jsval_);
    }

    // comparision
    bool operator ==(bool val) const {
        return bool(*this) == val;
    }
    bool operator !=(bool val) const {
        return bool(*this) != val;
    }

    static bool check(JSValueRef jsval) {
        return JSNIIsBoolean(env, jsval);
    }
};
// mimic the user-defined literal which doesn't support bool type
#define true_js JSBoolean(true)
#define false_js JSBoolean(false)


class JSNumber final : public JSValue {
public:
    JSNumber(JSValueRef jsval);
    explicit JSNumber(JSValueRef jsval, JSNumber defval):
        JSValue(jsval) {
        if (!check(jsval_))  jsval_ = defval;
    }

    JSNumber(): JSValue(0) {}
    template <typename T, typename = typename
              std::enable_if<std::is_arithmetic<T>::value>::type>
    JSNumber(T value):
        JSValue(JSNINewNumber(env, static_cast<double>(value))) {}

    // conversion
    template <typename T, typename = typename
              std::enable_if<std::is_arithmetic<T>::value>::type>
    operator T() const {
        return static_cast<T>(JSNIToCDouble(env, jsval_));
    }

    // assignment
    JSNumber& operator =(double val) {
        jsval_ = JSNINewNumber(env, val);
        return *this;
    }
    JSNumber& operator +=(double val) {
        return *this = (double)*this + val;
    }
    JSNumber& operator -=(double val) {
        return *this = (double)*this - val;
    }
    JSNumber& operator *=(double val) {
        return *this = (double)*this * val;
    }
    JSNumber& operator /=(double val) {
        return *this = (double)*this / val;
    }

    // increment / decrement
    JSNumber& operator ++() {
        return *this += 1;
    }
    JSNumber operator ++(int) {
        double val = *this;
        *this += 1;
        return val;
    }
    JSNumber& operator --() {
        return *this -= 1;
    }
    JSNumber operator --(int) {
        double val = *this;
        *this -= 1;
        return val;
    }

    bool isInteger() const {
        double num = *this;
        return modf(num, &num) == 0.0;
    }
    bool isNaN() const {
        return isnan((double)*this);
    }
    bool isFinite() const {
        return std::isfinite((double)*this);
    }

    static JSNumber parseInt(const std::string& string,
                             unsigned int radix = 10) {
        const char *b = string.c_str();
        char* e;
        double num = strtoll(b, &e, radix);
        if (errno == ERANGE)
            num = copysign(INFINITY, num);
        return e > b ? num : NAN;
    }
    static JSNumber parseFloat(const std::string& string) {
        double num = strtod(string.c_str(), NULL);
        if (std::abs(num) == HUGE_VAL)
            num = copysign(INFINITY, num);
        return num;
    }

    static bool check(JSValueRef jsval) {
        return JSNIIsNumber(env, jsval);
    }
};

// arithmetic
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
double operator +(const JSNumber& jsnum, T num) {
    return (double)jsnum + (double)num;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
double operator +(T num, const JSNumber& jsnum) {
    return (double)num + (double)jsnum;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
double operator -(const JSNumber& jsnum, T num) {
    return (double)jsnum - (double)num;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
double operator -(T num, const JSNumber& jsnum) {
    return (double)num - (double)jsnum;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
double operator *(const JSNumber& jsnum, T num) {
    return (double)jsnum * (double)num;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
double operator *(T num, const JSNumber& jsnum) {
    return (double)num * (double)jsnum;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
double operator /(const JSNumber& jsnum, T num) {
    return (double)jsnum / (double)num;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
double operator /(T num, const JSNumber& jsnum) {
    return (double)num / (double)jsnum;
}

// comparision
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator ==(const JSNumber& jsnum, T num) {
    return (double)jsnum == (double)num;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator ==(T num, const JSNumber& jsnum) {
    return (double)num == (double)jsnum;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator !=(const JSNumber& jsnum, T num) {
    return (double)jsnum != (double)num;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator !=(T num, const JSNumber& jsnum) {
    return (double)num != (double)jsnum;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator <=(const JSNumber& jsnum, T num) {
    return (double)jsnum <= (double)num;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator <=(T num, const JSNumber& jsnum) {
    return (double)num <= (double)jsnum;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator >=(const JSNumber& jsnum, T num) {
    return (double)jsnum >= (double)num;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator >=(T num, const JSNumber& jsnum) {
    return (double)num >= (double)jsnum;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator <(const JSNumber& jsnum, T num) {
    return (double)jsnum < (double)num;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator <(T num, const JSNumber& jsnum) {
    return (double)num < (double)jsnum;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator >(const JSNumber& jsnum, T num) {
    return (double)jsnum > (double)num;
}
template <typename T, typename = typename
          std::enable_if<std::is_arithmetic<T>::value>::type>
bool operator >(T num, const JSNumber& jsnum) {
    return (double)num > (double)jsnum;
}

// literal
inline JSNumber operator "" _js(long double num) {
    return JSNumber(num);
}
inline JSNumber operator "" _js(unsigned long long num) {
    return JSNumber((double)num);
}


class JSString final : public JSValue {
public:
    JSString(JSValueRef jsval);
    explicit JSString(JSValueRef jsval, JSString defval):
        JSValue(jsval) {
        if (!check(jsval_))  jsval_ = defval;
    }

    JSString(const std::string& str = std::string()):
        JSValue(JSNINewStringFromUtf8(env, str.c_str(), str.length())){}
    JSString(const char* str, size_t len = -1):
        JSValue(JSNINewStringFromUtf8(env, str, len)){}

    // conversion
    operator std::string() const {
        size_t len = length();
        std::string str(len, '\0');
        if (len > 0) {
            char* buf = const_cast<char*>(str.data());
            JSNIGetStringUtf8Chars(env, jsval_, buf, len);
        }
        return str;
    }

    // assignment
    JSString& operator +=(const std::string& str) {
        jsval_ = JSString(std::string(*this) + str);
        return *this;
    }

    size_t length() const {
        return JSNIGetStringUtf8Length(env, jsval_);
    }

    static bool check(JSValueRef jsval) {
        return JSNIIsString(env, jsval);
    }
};

// concatenation
inline std::string operator +(const JSString& jsstr, const std::string& str) {
    return (std::string)jsstr + str;
}
inline std::string operator +(const std::string& str, const JSString& jsstr) {
    return str + (std::string)jsstr;
}

// comparision
inline bool operator ==(const JSString& jsstr, const std::string& str) {
    return jsstr.length() == str.length() && std::string(jsstr) == str;
}
inline bool operator ==(const std::string& str, const JSString& jsstr) {
    return jsstr == str;
}
inline bool operator !=(const JSString& jsstr, const std::string& str) {
    return !(jsstr == str);
}
inline bool operator !=(const std::string& str, const JSString& jsstr) {
    return jsstr != str;
}

// literal
inline JSString operator "" _js(const char * str) {
    return JSString(str);
}
inline JSString operator "" _js(const char * str, std::size_t size) {
    return JSString(str);
}


class JSSymbol final : public JSValue {
public:
    JSSymbol(JSValueRef jsval): JSValue(jsval) {
        if (!check(jsval_))  jsval_ = JSNINewSymbol(env, jsval);
    }
    explicit JSSymbol(JSValueRef jsval, JSSymbol defval):
        JSValue(jsval) {
        if (!check(jsval_))  jsval_ = defval;
    }

    JSSymbol(const std::string& str = std::string()):
        JSValue(JSNINewSymbol(env, JSString(str))) {};
    JSSymbol(const char* str, size_t len = -1):
        JSValue(JSNINewSymbol(env, JSString(str, len))) {};

    static bool check(JSValueRef jsval) {
        return JSNIIsSymbol(env, jsval);
    }
};

}
