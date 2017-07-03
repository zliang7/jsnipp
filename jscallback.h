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

#include <unistd.h>
#include <sys/syscall.h>

#include <functional>
#include <future>
#include <tuple>

#include "apply.h"
#include "jsobject.h"
#include "jsfunction.h"

//#include <JSNIHelper.h>
extern "C" {
typedef void (*AsyncThreadWorkCallback)(JSNIEnv* env, void*);
typedef void (*AsyncThreadWorkAfterCallback)(JSNIEnv* env, void*);
void AsyncThreadWork(JSNIEnv* env, void* data,
                     AsyncThreadWorkCallback work,
                     AsyncThreadWorkAfterCallback callback);
}

namespace jsnipp {

class JSCallbackBase {
public:
    virtual ~JSCallbackBase() = default;
    operator bool() const {
        return jsfunc_ != nullptr;
    }
    operator JSFunction() const {
        return JSFunction(jsfunc_);
    }

protected:
    JSCallbackBase(): jsfunc_(nullptr) {}
    JSCallbackBase(JSFunction jsfunc, bool disposable):
        jsfunc_(jsfunc? jsfunc: nullptr), disposable_(disposable) {}

    static bool is_safe() {
        return syscall(SYS_gettid) == getpid();
    }

    //JSGlobalValue jsobj_;
    JSGlobalValue jsfunc_;
    bool disposable_;
};

template<typename R, typename ...Ts>
class JSUnsafeCallback : public JSCallbackBase {
public:
    JSUnsafeCallback(): JSCallbackBase() {};
    JSUnsafeCallback(JSFunction jsfunc, bool disposable = true):
        JSCallbackBase(jsfunc, disposable) {}

    R operator()(Ts&... args) {
        assert(is_safe());
        R result = call(std::forward_as_tuple(args...));
        if (disposable_)  delete this;
        return result;
    }

protected:
    virtual JSValue jsnify(Ts&... args) const {
        return JSUndefined();
    }

    R call(const std::tuple<Ts...>& tuple) const {
        auto jsnify = [this](Ts... args)->JSValue{
            return this->jsnify(args...);
        };
        JSValue argv = std::apply(jsnify, tuple);

        JSFunction jsfunc(jsfunc_);
        if (!argv.is_array())
            return static_cast<R>(jsfunc(argv));
        return static_cast<R>(jsfunc.apply(nullptr, JSArray(argv)));
    }
};

template<typename R, typename ...Ts>
class JSCallback : public JSUnsafeCallback<R, Ts...> {
public:
    using JSUnsafeCallback<R, Ts...>::JSUnsafeCallback;

    R operator()(Ts&... args) {
        if (JSUnsafeCallback<R, Ts...>::is_safe())
            return call(std::forward_as_tuple(args...));

        args_ = std::make_tuple(args...);
        AsyncThreadWork(NULL, this, [](JSNIEnv*, void*){}, callback);
        R result = result_.get_future().get();
        if (this->disposable_)  delete this;
        return result;
    }

private:
    static void callback(JSNIEnv* env, void* data) {
        auto self = reinterpret_cast<JSCallback<R, Ts...>*>(data);
        self->result_.set_value(self->call(self->args_));
    }
    std::tuple<Ts...> args_;
    std::promise<R> result_;
};

template<typename ...Ts>
class JSCallback<void, Ts...> : public JSUnsafeCallback<void, Ts...> {
public:
    using JSUnsafeCallback<void, Ts...>::JSUnsafeCallback;

    void operator()(Ts&... args) {
        args_ = std::make_tuple(args...);
        AsyncThreadWork(NULL, this, [](JSNIEnv*, void*){}, callback);
    }

private:
    static void callback(JSNIEnv* env, void* data) {
        auto self = reinterpret_cast<JSCallback<void, Ts...>*>(data);
        self->call(self->args_);
        if (self->disposable_)  delete self;
    }
    std::tuple<Ts...> args_;
};

}
