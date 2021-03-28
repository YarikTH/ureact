#pragma once

#include "ureact/detail/signal.hpp"
#include "ureact/detail/graph/function_op.hpp"

namespace ureact
{

namespace detail
{

template <typename S, typename op_t, typename F, typename arg_node_t>
auto make_unary_operator_signal( context& context, arg_node_t&& arg_node ) -> temp_signal<S, op_t>
{
    return temp_signal<S, op_t>( std::make_shared<signal_op_node<S, op_t>>(
        context, F(), std::forward<arg_node_t>( arg_node ) ) );
}

template <template <typename> class functor_op,
    typename signal_t,
    typename val_t = typename signal_t::value_t,
    class = typename std::enable_if<is_signal<signal_t>::value>::type,
    typename F = functor_op<val_t>,
    typename S = typename std::result_of<F( val_t )>::type,
    typename op_t = function_op<S, F, signal_node_ptr_t<val_t>>>
auto unary_operator_impl( const signal_t& arg ) -> temp_signal<S, op_t>
{
    return make_unary_operator_signal<S, op_t, F>( arg.get_context(), get_node_ptr( arg ) );
}

template <template <typename> class functor_op,
    typename val_t,
    typename op_in_t,
    typename F = functor_op<val_t>,
    typename S = typename std::result_of<F( val_t )>::type,
    typename op_t = function_op<S, F, op_in_t>>
auto unary_operator_impl( temp_signal<val_t, op_in_t>&& arg ) -> temp_signal<S, op_t>
{
    return make_unary_operator_signal<S, op_t, F>( arg.get_context(), arg.steal_op() );
}

} // namespace detail

#define UREACT_DECLARE_UNARY_OP_FUNCTOR( op, name )                                                \
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
    } /* namespace op_functors */                                                                  \
    } /* namespace detail */

#define UREACT_DECLARE_UNARY_OP( op, name )                                                        \
    template <typename arg_t,                                                                      \
        template <typename> class functor_op = detail::op_functors::op_functor_##name>             \
    auto operator op( arg_t&& arg )                                                                \
        ->decltype( detail::unary_operator_impl<functor_op>( std::forward<arg_t>( arg ) ) )        \
    {                                                                                              \
        return detail::unary_operator_impl<functor_op>( std::forward<arg_t>( arg ) );              \
    }

#define UREACT_DECLARE_UNARY_OPERATOR( op, name )                                                  \
    UREACT_DECLARE_UNARY_OP_FUNCTOR( op, name )                                                    \
    UREACT_DECLARE_UNARY_OP( op, name )

UREACT_DECLARE_UNARY_OPERATOR( +, unary_plus )
UREACT_DECLARE_UNARY_OPERATOR( -, unary_minus )
UREACT_DECLARE_UNARY_OPERATOR( !, logical_negation )
UREACT_DECLARE_UNARY_OPERATOR( ~, bitwise_complement )

#undef UREACT_DECLARE_UNARY_OPERATOR

#define UREACT_DECLARE_BINARY_OP_FUNCTOR( op, name )                                               \
    namespace detail                                                                               \
    {                                                                                              \
    namespace op_functors                                                                          \
    {                                                                                              \
    template <typename L, typename R>                                                              \
    struct op_functor_##name                                                                       \
    {                                                                                              \
        auto operator()( const L& lhs, const R& rhs ) const                                        \
            -> decltype( std::declval<L>() op std::declval<R>() )                                  \
        {                                                                                          \
            return lhs op rhs;                                                                     \
        }                                                                                          \
    };                                                                                             \
    } /* namespace op_functors */                                                                  \
    } /* namespace detail */

#define UREACT_DECLARE_BINARY_OP_R_FUNCTOR( op, name )                                             \
    namespace detail                                                                               \
    {                                                                                              \
    namespace op_functors                                                                          \
    {                                                                                              \
    template <typename L, typename R>                                                              \
    struct op_r_functor_##name                                                                     \
    {                                                                                              \
        op_r_functor_##name( op_r_functor_##name&& other ) noexcept                                \
            : m_left_val( std::move( other.m_left_val ) )                                          \
        {}                                                                                         \
                                                                                                   \
        op_r_functor_##name& operator=( op_r_functor_##name&& other ) noexcept = delete;           \
                                                                                                   \
        template <typename T,                                                                      \
            class = typename std::enable_if<!is_same_decay<T, op_r_functor_##name>::value>::type>  \
        explicit op_r_functor_##name( T&& val )                                                    \
            : m_left_val( std::forward<T>( val ) )                                                 \
        {}                                                                                         \
                                                                                                   \
        op_r_functor_##name( const op_r_functor_##name& other ) = delete;                          \
                                                                                                   \
        op_r_functor_##name& operator=( const op_r_functor_##name& other ) = delete;               \
                                                                                                   \
        ~op_r_functor_##name() = default;                                                          \
                                                                                                   \
        auto operator()( const R& rhs ) const                                                      \
            -> decltype( std::declval<L>() op std::declval<R>() )                                  \
        {                                                                                          \
            return m_left_val op rhs;                                                              \
        }                                                                                          \
                                                                                                   \
        L m_left_val;                                                                              \
    };                                                                                             \
    } /* namespace op_functors */                                                                  \
    } /* namespace detail */

#define UREACT_DECLARE_BINARY_OP_L_FUNCTOR( op, name )                                             \
    namespace detail                                                                               \
    {                                                                                              \
    namespace op_functors                                                                          \
    {                                                                                              \
    template <typename L, typename R>                                                              \
    struct op_l_functor_##name                                                                     \
    {                                                                                              \
        op_l_functor_##name( op_l_functor_##name&& other ) noexcept                                \
            : m_right_val( std::move( other.m_right_val ) )                                        \
        {}                                                                                         \
                                                                                                   \
        op_l_functor_##name& operator=( op_l_functor_##name&& other ) noexcept = delete;           \
                                                                                                   \
        template <typename T,                                                                      \
            class = typename std::enable_if<!is_same_decay<T, op_l_functor_##name>::value>::type>  \
        explicit op_l_functor_##name( T&& val )                                                    \
            : m_right_val( std::forward<T>( val ) )                                                \
        {}                                                                                         \
                                                                                                   \
        op_l_functor_##name( const op_l_functor_##name& other ) = delete;                          \
                                                                                                   \
        op_l_functor_##name& operator=( const op_l_functor_##name& other ) = delete;               \
                                                                                                   \
        ~op_l_functor_##name() = default;                                                          \
                                                                                                   \
        auto operator()( const L& lhs ) const                                                      \
            -> decltype( std::declval<L>() op std::declval<R>() )                                  \
        {                                                                                          \
            return lhs op m_right_val;                                                             \
        }                                                                                          \
                                                                                                   \
        R m_right_val;                                                                             \
    };                                                                                             \
    } /* namespace op_functors */                                                                  \
    } /* namespace detail */

#define UREACT_DECLARE_BINARY_OP_1_SIGNALS( op, name )                                                     \
    template <typename left_signal_t,                                                              \
        typename right_signal_t,                                                                   \
        typename left_val_t = typename left_signal_t::value_t,                                     \
        typename right_val_t = typename right_signal_t::value_t,                                   \
        class = typename std::enable_if<is_signal<left_signal_t>::value>::type,                    \
        class = typename std::enable_if<is_signal<right_signal_t>::value>::type,                   \
        typename F = detail::op_functors::op_functor_##name<left_val_t, right_val_t>,              \
        typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,                  \
        typename op_t = detail::function_op<S,                                                     \
            F,                                                                                     \
            detail::signal_node_ptr_t<left_val_t>,                                                 \
            detail::signal_node_ptr_t<right_val_t>>>                                               \
    auto operator op( const left_signal_t& lhs, const right_signal_t& rhs )                        \
        ->detail::temp_signal<S, op_t>                                                             \
    {                                                                                              \
        context& context = lhs.get_context();                                                      \
        assert( context == rhs.get_context() );                                                    \
        return detail::temp_signal<S, op_t>( std::make_shared<detail::signal_op_node<S, op_t>>(    \
            context, F(), get_node_ptr( lhs ), get_node_ptr( rhs ) ) );                            \
    }

#define UREACT_DECLARE_BINARY_OP_2_SIGNAL_VALUE( op, name )                                                     \
    template <typename left_signal_t,                                                              \
        typename right_val_in_t,                                                                   \
        typename left_val_t = typename left_signal_t::value_t,                                     \
        typename right_val_t = typename std::decay<right_val_in_t>::type,                          \
        class = typename std::enable_if<is_signal<left_signal_t>::value>::type,                    \
        class = typename std::enable_if<!is_signal<right_val_t>::value>::type,                     \
        typename F = detail::op_functors::op_l_functor_##name<left_val_t, right_val_t>,            \
        typename S = typename std::result_of<F( left_val_t )>::type,                               \
        typename op_t = detail::function_op<S, F, detail::signal_node_ptr_t<left_val_t>>>          \
    auto operator op( const left_signal_t& lhs, right_val_in_t&& rhs )                             \
        ->detail::temp_signal<S, op_t>                                                             \
    {                                                                                              \
        context& context = lhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>( std::make_shared<detail::signal_op_node<S, op_t>>(    \
            context, F( std::forward<right_val_in_t>( rhs ) ), get_node_ptr( lhs ) ) );            \
    }

#define UREACT_DECLARE_BINARY_OP_3_VALUE_SIGNAL( op, name )                                                     \
    template <typename left_val_in_t,                                                              \
        typename right_signal_t,                                                                   \
        typename left_val_t = typename std::decay<left_val_in_t>::type,                            \
        typename right_val_t = typename right_signal_t::value_t,                                   \
        class = typename std::enable_if<!is_signal<left_val_t>::value>::type,                      \
        class = typename std::enable_if<is_signal<right_signal_t>::value>::type,                   \
        typename F = detail::op_functors::op_r_functor_##name<left_val_t, right_val_t>,            \
        typename S = typename std::result_of<F( right_val_t )>::type,                              \
        typename op_t = detail::function_op<S, F, detail::signal_node_ptr_t<right_val_t>>>         \
    auto operator op( left_val_in_t&& lhs, const right_signal_t& rhs )                             \
        ->detail::temp_signal<S, op_t>                                                             \
    {                                                                                              \
        context& context = rhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>(                                              \
            std::make_shared<detail::signal_op_node<S, op_t>>(                                     \
                context, F( std::forward<left_val_in_t>( lhs ) ), get_node_ptr( rhs ) ) );         \
    }

#define UREACT_DECLARE_BINARY_OP_4_TEMPS( op, name )                                           \
    template <typename left_val_t,                                                                 \
        typename left_op_t,                                                                        \
        typename right_val_t,                                                                      \
        typename right_op_t,                                                                       \
        typename F = detail::op_functors::op_functor_##name<left_val_t, right_val_t>,              \
        typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,                  \
        typename op_t = detail::function_op<S, F, left_op_t, right_op_t>>                          \
    auto operator op( detail::temp_signal<left_val_t, left_op_t>&& lhs,                            \
        detail::temp_signal<right_val_t, right_op_t>&& rhs )                                       \
        ->detail::temp_signal<S, op_t>                                                             \
    {                                                                                              \
        context& context = lhs.get_context();                                                      \
        assert( context == rhs.get_context() );                                                    \
        return detail::temp_signal<S, op_t>(                                                       \
            std::make_shared<detail::signal_op_node<S, op_t>>(                                     \
                context, F(), lhs.steal_op(), rhs.steal_op() ) );                                  \
    }

#define UREACT_DECLARE_BINARY_OP_5_TEMP_SIGNAL( op, name )                                                     \
    template <typename left_val_t,                                                                 \
        typename left_op_t,                                                                        \
        typename right_signal_t,                                                                   \
        typename right_val_t = typename right_signal_t::value_t,                                   \
        class = typename std::enable_if<is_signal<right_signal_t>::value>::type,                   \
        typename F = detail::op_functors::op_functor_##name<left_val_t, right_val_t>,              \
        typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,                  \
        typename op_t                                                                              \
        = detail::function_op<S, F, left_op_t, detail::signal_node_ptr_t<right_val_t>>>            \
    auto operator op(                                                                              \
        detail::temp_signal<left_val_t, left_op_t>&& lhs, const right_signal_t& rhs )              \
        ->detail::temp_signal<S, op_t>                                                             \
    {                                                                                              \
        context& context = rhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>( std::make_shared<detail::signal_op_node<S, op_t>>(    \
            context, F(), lhs.steal_op(), get_node_ptr( rhs ) ) );                                 \
    }

#define UREACT_DECLARE_BINARY_OP_6_SIGNAL_TEMP( op, name )                                                     \
    template <typename left_signal_t,                                                              \
        typename right_val_t,                                                                      \
        typename right_op_t,                                                                       \
        typename left_val_t = typename left_signal_t::value_t,                                     \
        class = typename std::enable_if<is_signal<left_signal_t>::value>::type,                    \
        typename F = detail::op_functors::op_functor_##name<left_val_t, right_val_t>,              \
        typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,                  \
        typename op_t                                                                              \
        = detail::function_op<S, F, detail::signal_node_ptr_t<left_val_t>, right_op_t>>            \
    auto operator op(                                                                              \
        const left_signal_t& lhs, detail::temp_signal<right_val_t, right_op_t>&& rhs )             \
        ->detail::temp_signal<S, op_t>                                                             \
    {                                                                                              \
        context& context = lhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>( std::make_shared<detail::signal_op_node<S, op_t>>(    \
            context, F(), get_node_ptr( lhs ), rhs.steal_op() ) );                                 \
    }

#define UREACT_DECLARE_BINARY_OP_7_TEMP_VALUE( op, name )                                                     \
    template <typename left_val_t,                                                                 \
        typename left_op_t,                                                                        \
        typename right_val_in_t,                                                                   \
        typename right_val_t = typename std::decay<right_val_in_t>::type,                          \
        class = typename std::enable_if<!is_signal<right_val_t>::value>::type,                     \
        typename F = detail::op_functors::op_l_functor_##name<left_val_t, right_val_t>,            \
        typename S = typename std::result_of<F( left_val_t )>::type,                               \
        typename op_t = detail::function_op<S, F, left_op_t>>                                      \
    auto operator op( detail::temp_signal<left_val_t, left_op_t>&& lhs, right_val_in_t&& rhs )     \
        ->detail::temp_signal<S, op_t>                                                             \
    {                                                                                              \
        context& context = lhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>( std::make_shared<detail::signal_op_node<S, op_t>>(    \
            context, F( std::forward<right_val_in_t>( rhs ) ), lhs.steal_op() ) );                 \
    }

#define UREACT_DECLARE_BINARY_OP_8_VALUE_TEMP( op, name )                                                     \
    template <typename left_val_in_t,                                                              \
        typename right_val_t,                                                                      \
        typename right_op_t,                                                                       \
        typename left_val_t = typename std::decay<left_val_in_t>::type,                            \
        class = typename std::enable_if<!is_signal<left_val_t>::value>::type,                      \
        typename F = detail::op_functors::op_r_functor_##name<left_val_t, right_val_t>,            \
        typename S = typename std::result_of<F( right_val_t )>::type,                              \
        typename op_t = detail::function_op<S, F, right_op_t>>                                     \
    auto operator op( left_val_in_t&& lhs, detail::temp_signal<right_val_t, right_op_t>&& rhs )    \
        ->detail::temp_signal<S, op_t>                                                             \
    {                                                                                              \
        context& context = rhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>(                                              \
            std::make_shared<detail::signal_op_node<S, op_t>>(                                     \
                context, F( std::forward<left_val_in_t>( lhs ) ), rhs.steal_op() ) );              \
    }

#define UREACT_DECLARE_BINARY_OPERATOR( op, name )                                                 \
    UREACT_DECLARE_BINARY_OP_FUNCTOR( op, name )                                                   \
    UREACT_DECLARE_BINARY_OP_R_FUNCTOR( op, name )                                                 \
    UREACT_DECLARE_BINARY_OP_L_FUNCTOR( op, name )                                                 \
    UREACT_DECLARE_BINARY_OP_1_SIGNALS( op, name )                                                 \
    UREACT_DECLARE_BINARY_OP_2_SIGNAL_VALUE( op, name )                                            \
    UREACT_DECLARE_BINARY_OP_3_VALUE_SIGNAL( op, name )                                            \
    UREACT_DECLARE_BINARY_OP_4_TEMPS( op, name )                                                   \
    UREACT_DECLARE_BINARY_OP_5_TEMP_SIGNAL( op, name )                                             \
    UREACT_DECLARE_BINARY_OP_6_SIGNAL_TEMP( op, name )                                             \
    UREACT_DECLARE_BINARY_OP_7_TEMP_VALUE( op, name )                                              \
    UREACT_DECLARE_BINARY_OP_8_VALUE_TEMP( op, name )

UREACT_DECLARE_BINARY_OPERATOR( +, addition )
UREACT_DECLARE_BINARY_OPERATOR( -, subtraction )
UREACT_DECLARE_BINARY_OPERATOR( *, multiplication )
UREACT_DECLARE_BINARY_OPERATOR( /, division )
UREACT_DECLARE_BINARY_OPERATOR( %, modulo )

UREACT_DECLARE_BINARY_OPERATOR( ==, equal )
UREACT_DECLARE_BINARY_OPERATOR( !=, not_equal )
UREACT_DECLARE_BINARY_OPERATOR( <, less )
UREACT_DECLARE_BINARY_OPERATOR( <=, less_equal )
UREACT_DECLARE_BINARY_OPERATOR( >, greater )
UREACT_DECLARE_BINARY_OPERATOR( >=, greater_equal )

UREACT_DECLARE_BINARY_OPERATOR( &&, logical_and )
UREACT_DECLARE_BINARY_OPERATOR( ||, logical_or )

UREACT_DECLARE_BINARY_OPERATOR( &, bitwise_and )
UREACT_DECLARE_BINARY_OPERATOR( |, bitwise_or )
UREACT_DECLARE_BINARY_OPERATOR( ^, bitwise_xor )
UREACT_DECLARE_BINARY_OPERATOR( <<, bitwise_left_shift )
UREACT_DECLARE_BINARY_OPERATOR( >>, bitwise_right_shift )

#undef UREACT_DECLARE_BINARY_OPERATOR

} // namespace ureact
