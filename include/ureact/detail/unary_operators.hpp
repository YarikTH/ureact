#pragma once

#include "ureact/detail/signal.hpp"
#include "ureact/detail/graph/function_op.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Unary operators
///////////////////////////////////////////////////////////////////////////////////////////////////
#define REACT_DECLARE_OP( op, name )                                                               \
                                                                                                   \
    namespace ureact                                                                               \
    {                                                                                              \
    namespace detail                                                                               \
    {                                                                                              \
    namespace op_functors                                                                          \
    {                                                                                              \
    template <typename T>                                                                          \
    struct op_functor_##name                                                                       \
    {                                                                                              \
        auto operator()( const T& v ) const -> decltype( op std::declval<T>() )                    \
        {                                                                                          \
            return op v;                                                                           \
        }                                                                                          \
    };                                                                                             \
    }                                                                                              \
    }                                                                                              \
    }                                                                                              \
                                                                                                   \
    namespace ureact                                                                               \
    {                                                                                              \
    template <typename signal_t,                                                                   \
        typename val_t = typename signal_t::value_t,                                               \
        class = typename std::enable_if<is_signal<signal_t>::value>::type,                         \
        typename F = ::ureact::detail::op_functors::op_functor_##name<val_t>,                      \
        typename S = typename std::result_of<F( val_t )>::type,                                    \
        typename op_t                                                                              \
        = ::ureact::detail::function_op<S, F, ::ureact::detail::signal_node_ptr_t<val_t>>>         \
    auto operator op( const signal_t& arg ) -> detail::temp_signal<S, op_t>                        \
    {                                                                                              \
        context& context = arg.get_context();                                                      \
        return detail::temp_signal<S, op_t>(                                                       \
            std::make_shared<::ureact::detail::signal_op_node<S, op_t>>(                           \
                context, F(), get_node_ptr( arg ) ) );                                             \
    }                                                                                              \
                                                                                                   \
    template <typename val_t,                                                                      \
        typename op_in_t,                                                                          \
        typename F = ::ureact::detail::op_functors::op_functor_##name<val_t>,                      \
        typename S = typename std::result_of<F( val_t )>::type,                                    \
        typename op_t = ::ureact::detail::function_op<S, F, op_in_t>>                              \
    auto operator op( detail::temp_signal<val_t, op_in_t>&& arg ) -> detail::temp_signal<S, op_t>  \
    {                                                                                              \
        context& context = arg.get_context();                                                      \
        return detail::temp_signal<S, op_t>( context,                                              \
            std::make_shared<::ureact::detail::signal_op_node<S, op_t>>(                           \
                context, F(), arg.steal_op() ) );                                                  \
    }                                                                                              \
    }

REACT_DECLARE_OP( +, unary_plus )
REACT_DECLARE_OP( -, unary_minus )
REACT_DECLARE_OP( !, logical_negation )
REACT_DECLARE_OP( ~, bitwise_complement )

#undef REACT_DECLARE_OP
