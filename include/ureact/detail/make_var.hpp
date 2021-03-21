#pragma once

#include "ureact/detail/signal.hpp"

namespace ureact
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// make_var
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename V,
    typename S = typename std::decay<V>::type,
    class = typename std::enable_if<!is_signal<S>::value>::type>
auto make_var( context* context, V&& value ) -> var_signal<S>
{
    return var_signal<S>(
        std::make_shared<::ureact::detail::var_node<S>>( context, std::forward<V>( value ) ) );
}

template <typename S>
auto make_var( context* context, std::reference_wrapper<S> value ) -> var_signal<S&>
{
    return var_signal<S&>(
        std::make_shared<::ureact::detail::var_node<std::reference_wrapper<S>>>( context, value ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// make_var (higher order reactives)
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename V,
    typename S = typename std::decay<V>::type,
    typename inner_t = typename S::value_t,
    class = typename std::enable_if<is_signal<S>::value>::type>
auto make_var( context* context, V&& value ) -> var_signal<signal<inner_t>>
{
    return var_signal<signal<inner_t>>(
        std::make_shared<::ureact::detail::var_node<signal<inner_t>>>(
            context, std::forward<V>( value ) ) );
}

} // namespace ureact
