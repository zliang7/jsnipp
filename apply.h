#pragma once

#include <tuple>

#if __cplusplus < 201406L //201703L

#include <type_traits>
#include <utility>

namespace std {

#if __cplusplus >= 201402L  // C++14

template<typename F, typename Tuple, size_t ...I >
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

#if !defined(__cpp_lib_integer_sequence)
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

template<typename F, typename Tuple, size_t ...I >
constexpr jsni::JSValue apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>) {
    return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}
template<typename F, typename Tuple>
constexpr jsni::JSValue apply(F&& f, Tuple&& t) {
    return apply_impl(std::forward<F>(f), std::forward<Tuple>(t),
                      std::make_index_sequence<std::tuple_size<
                              typename std::decay<Tuple>::type>::value>());
}

#endif

}
#endif
