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

#if !defined(__cpp_generic_lambdas)
#error "C++ generic lambdas support requires G++ 4.9 / clang 3.4 or later"
#endif

namespace jsni {

class JSCallbackBase {
public:
    virtual ~JSCallbackBase() = default;
    explicit operator bool() const {
        return (bool)jsfunc_;
    }
    operator JSFunction() const {
        return JSFunction(jsfunc_);
    }

protected:
    JSCallbackBase(JSGlobalValue jsfunc = nullptr): jsfunc_(jsfunc) {}

    static bool is_safe() {
        return syscall(SYS_gettid) == getpid();
    }

    //JSGlobalValue jsobj_;
    JSGlobalValue jsfunc_;
};

template<typename R, typename ...Ts>
class JSUnsafeCallback : public JSCallbackBase {
public:
    JSUnsafeCallback(JSGlobalValue jsfunc = nullptr):
        JSCallbackBase(jsfunc) {}
    JSUnsafeCallback(const JSUnsafeCallback& that):
        JSCallbackBase(that.jsfunc_) {}
    JSUnsafeCallback(JSFunction jsfunc):
        JSCallbackBase(jsfunc? JSGlobalValue(jsfunc): nullptr) {}

    template <typename ...Us>
    R operator()(Us&&... args) {
        assert(is_safe());
        return call(std::forward<Us>(args)...);
    }

protected:
    virtual JSValue jsnify(Ts&... args) const {
        return JSValue();
    }

    template <typename ...Us>
    R call(Us&&... args) const {
        JSValue argv = jsnify(std::forward<Us>(args)...);
        JSFunction jsfunc(jsfunc_);
        if (!argv.is(Valid))
            return static_cast<R>(jsfunc());
        if (!argv.is(Array))
            return static_cast<R>(jsfunc(argv));
        return static_cast<R>(jsfunc.apply(nullptr, argv.as(Array)));
    }
};

template<typename R, typename ...Ts>
class JSCallback : public JSUnsafeCallback<R, Ts...> {
public:
    using JSUnsafeCallback<R, Ts...>::JSUnsafeCallback;

    template<typename ...Us>
    R operator()(Us&&... args) {
        if (JSUnsafeCallback<R, Ts...>::is_safe())
            return call(std::forward<Us>(args)...);

        args_ = std::move(std::make_tuple(args...));
        AsyncThreadWork(NULL, this, [](JSNIEnv*, void*){}, callback);
        return result_.get_future().get();
    }

private:
    static void callback(JSNIEnv* env, void* data) {
        auto self = reinterpret_cast<JSCallback<R, Ts...>*>(data);
        auto call = [self](auto&&... args) -> JSValue {
            return self->call(std::forward<decltype(args)>(args)...);
        };
        self->result_.set_value(jsni::apply(call, self->args_));
    }
    std::tuple<Ts...> args_;
    std::promise<R> result_;
};

template<typename ...Ts>
class JSCallback<void, Ts...> : public JSUnsafeCallback<void, Ts...> {
public:
    using JSUnsafeCallback<void, Ts...>::JSUnsafeCallback;

    template<typename ...Us>
    void operator()(Us&&... args) {
        assert(self_ == this);  // object must be allocated in heap
        args_ = std::make_tuple(args...);
        AsyncThreadWork(NULL, this, [](JSNIEnv*, void*){}, callback);
    }

#ifndef NDEBUG
    static void* operator new (std::size_t count) {
        void* addr = ::operator new(count);
        reinterpret_cast<JSCallback<void, Ts...>*>(addr)->self_ = addr;
        return addr;
    }
#endif

private:
    static void callback(JSNIEnv* env, void* data) {
        auto self = reinterpret_cast<JSCallback<void, Ts...>*>(data);
        auto call = [self](auto&&... args) {
            self->call(std::forward<decltype(args)>(args)...);
        };
        jsni::apply(call, self->args_);
        delete self;
    }
    std::tuple<Ts...> args_;
    void* self_;
};

}
