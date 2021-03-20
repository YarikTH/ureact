#pragma once

#include "ureact/detail/signal.hpp"
#include "ureact/detail/signal_pack.hpp"
#include "ureact/detail/graph/function_op.hpp"
#include "ureact/detail/graph/signal_op_node.hpp"

namespace ureact
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// make_signal
///////////////////////////////////////////////////////////////////////////////////////////////////
// Single arg
template
<
    typename value_t,
    typename in_f,
    typename F = typename std::decay<in_f>::type,
    typename S = typename std::result_of<F(value_t)>::type,
    typename op_t = ::ureact::detail::function_op<S,F, ::ureact::detail::signal_node_ptr_t<value_t>>
>
auto make_signal(const signal<value_t>& arg, in_f&& func)
    -> detail::temp_signal<S,op_t>
{
    context* context = arg.get_context();
    return detail::temp_signal<S,op_t>(
        std::make_shared<::ureact::detail::signal_op_node<S,op_t>>(
            context, std::forward<in_f>(func), get_node_ptr(arg)));
}

// Multiple args
template
<
    typename ... values_t,
    typename in_f,
    typename F = typename std::decay<in_f>::type,
    typename S = typename std::result_of<F(values_t...)>::type,
    typename op_t = ::ureact::detail::function_op<S,F, ::ureact::detail::signal_node_ptr_t<values_t> ...>
>
auto make_signal(const signal_pack<values_t...>& arg_pack, in_f&& func)
    -> detail::temp_signal<S,op_t>
{
    struct node_builder
    {
        explicit node_builder( context* context, in_f&& func )
            : m_context( context )
            , m_my_func( std::forward<in_f>( func ) )
        {}

        auto operator()( const signal<values_t>&... args ) -> detail::temp_signal<S, op_t>
        {
            return detail::temp_signal<S,op_t>(
                std::make_shared<::ureact::detail::signal_op_node<S,op_t>>(
                    m_context,
                    std::forward<in_f>( m_my_func ), get_node_ptr( args ) ...));
        }

        context* m_context;
        in_f m_my_func;
    };

    return apply(
        node_builder( std::get<0>(arg_pack.data).get_context(), std::forward<in_f>(func) ),
        arg_pack.data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// operator->* overload to connect signals to a function and return the resulting signal.
///////////////////////////////////////////////////////////////////////////////////////////////////
// Single arg
template
<
    typename F,
    template <typename V_> class signal_t,
    typename value_t,
    class = typename std::enable_if<
        is_signal<signal_t<value_t>>::value>::type
>
auto operator->*( const signal_t<value_t>& arg, F&& func )
    -> signal<typename std::result_of<F( value_t )>::type>
{
    return ::ureact::make_signal( arg, std::forward<F>( func ) );
}

// Multiple args
template <typename F, typename... values_t>
auto operator->*( const signal_pack<values_t...>& arg_pack, F&& func )
    -> signal<typename std::result_of<F( values_t... )>::type>
{
    return ::ureact::make_signal( arg_pack, std::forward<F>( func ) );
}

}
