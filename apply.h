#pragma once

#include <tuple>

#if defined(__cpp_lib_apply)

namespace jsni {

template<typename F, typename Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t) {
    return std::apply(std::forward<F>(f), std::forward<Tuple>(t));
}

}

#else

#include <type_traits>
#include <utility>

namespace jsni {

#if defined(__cpp_decltype_auto)
// https://isocpp.org/wiki/faq/cpp14-language#decltype-auto

template<typename F, typename Tuple, size_t ...I>
constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>) {
    return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}
template<typename F, typename Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t) {
    return apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
                      std::make_index_sequence<std::tuple_size<
                              typename std::decay<Tuple>::type>::value>());
}

#elif __cplusplus >= 201103L  // C++11

#if defined(__cpp_lib_integer_sequence)

template<std::size_t... Ints>
using index_sequence = std::index_sequence<Ints...>;
template<std::size_t N>
using make_index_sequence = std::make_index_sequence<N>;

#else

// ref: http://www.pdimov.com/cpp2/simple_cxx11_metaprogramming.html
template<class T, T... Ints>
struct integer_sequence {
};

template<std::size_t... Ints>
using index_sequence = integer_sequence<std::size_t, Ints...>;

namespace impl {
template<class S> struct next_integer_sequence;

template<class T, T... Ints>
struct next_integer_sequence<integer_sequence<T, Ints...>> {
    using type = integer_sequence<T, Ints..., sizeof...(Ints)>;
};

template<class T, T I, T N> struct make_int_seq_impl;

template<class T, T I, T N>
struct make_int_seq_impl {
    using type = typename next_integer_sequence<
        typename make_int_seq_impl<T, I+1, N>::type>::type;
};

template<class T, T N>
struct make_int_seq_impl<T, N, N> {
    using type = integer_sequence<T>;
};
}

template<class T, T N>
using make_integer_sequence = typename impl::make_int_seq_impl<T, 0, N>::type;

template<std::size_t N>
using make_index_sequence = make_integer_sequence<std::size_t, N>;

#endif

// functor
template<class F>
struct function_traits : public function_traits<decltype(&F::operator())> {
};
template<class F>
struct function_traits<F&> : public function_traits<decltype(&F::operator())> {
};

// free function
template<class R, class... Args>
struct function_traits<R(Args...)> {
    using return_type = R;

    static constexpr std::size_t arity = sizeof...(Args);

    template <std::size_t N>
    struct argument {
        static_assert(N < arity, "error: invalid parameter index.");
        using type = typename std::tuple_element<N,std::tuple<Args...>>::type;
    };
};
// free function pointer
template<class R, class... Args>
struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)> {
};
// free function reference
template<class R, class... Args>
struct function_traits<R(&)(Args...)> : public function_traits<R(Args...)> {
};

// member function pointer
template<class C, class R, class... Args>
struct function_traits<R(C::*)(Args...)> :
        public function_traits<R(C&,Args...)> {
};
// const member function pointer
template<class C, class R, class... Args>
struct function_traits<R(C::*)(Args...) const> :
        public function_traits<R(C&,Args...)> {
};
// member object pointer
template<class C, class R>
struct function_traits<R(C::*)> : public function_traits<R(C&)> {
};

template<typename F, typename Tuple, size_t ...I>
constexpr auto apply_impl(F&& f, Tuple&& t, index_sequence<I...>) ->
        typename function_traits<F>::return_type {
    return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}
template<typename F, typename Tuple>
constexpr auto apply(F&& f, Tuple&& t) ->
        typename function_traits<F>::return_type {
    return apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
                      make_index_sequence<std::tuple_size<
                              typename std::decay<Tuple>::type>::value>());
}

#else

#error "C++11 support is required"

#endif

}

#endif
