#pragma once

#include "ureact/detail/cpp_features_detection.hpp"
#include <tuple>

namespace ureact { namespace detail {

#ifdef REACT_HAS_CPP_17
    using std::apply;
#else

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Unpack tuple - see
/// http://stackoverflow.com/questions/687490/how-do-i-expand-a-tuple-into-variadic-template-functions-arguments
///////////////////////////////////////////////////////////////////////////////////////////////////

template<size_t N>
struct apply_helper {
    template<typename F, typename T, typename... A>
    static inline auto apply(F && f, T && t, A &&... a)
        -> decltype(apply_helper<N-1>::apply(std::forward<F>(f), std::forward<T>(t), std::get<N-1>(std::forward<T>(t)), std::forward<A>(a)...))
    {
        return apply_helper<N-1>::apply(std::forward<F>(f), std::forward<T>(t), std::get<N-1>(std::forward<T>(t)), std::forward<A>(a)...);
    }
};

template<>
struct apply_helper<0>
{
    template<typename F, typename T, typename... A>
    static inline auto apply(F && f, T && /*unused*/, A &&... a)
        -> decltype(std::forward<F>(f)(std::forward<A>(a)...))
    {
        return std::forward<F>(f)(std::forward<A>(a)...);
    }
};

template<typename F, typename T>
inline auto apply(F && f, T && t)
    -> decltype(apply_helper<std::tuple_size<typename std::decay<T>::type>::value>::apply(std::forward<F>(f), std::forward<T>(t)))
{
    return apply_helper<std::tuple_size<typename std::decay<T>::type>::value>
        ::apply(std::forward<F>(f), std::forward<T>(t));
}

#endif

}}
