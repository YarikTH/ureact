#pragma once

#include "ureact/detail/signal.hpp"
#include "ureact/detail/graph/function_op.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Unary operators
///////////////////////////////////////////////////////////////////////////////////////////////////
#define UREACT_DECLARE_UNARY_OPERATOR( op, name )                                                  \
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

UREACT_DECLARE_UNARY_OPERATOR( +, unary_plus )
UREACT_DECLARE_UNARY_OPERATOR( -, unary_minus )
UREACT_DECLARE_UNARY_OPERATOR( !, logical_negation )
UREACT_DECLARE_UNARY_OPERATOR( ~, bitwise_complement )

#undef UREACT_DECLARE_UNARY_OPERATOR

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Binary operators
///////////////////////////////////////////////////////////////////////////////////////////////////
#define UREACT_DECLARE_BINARY_OPERATOR( op, name )                                                 \
                                                                                                   \
    namespace ureact                                                                               \
    {                                                                                              \
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
                                                                                                   \
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
                                                                                                   \
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
    }                                                                                              \
    }                                                                                              \
    }                                                                                              \
                                                                                                   \
    namespace ureact                                                                               \
    {                                                                                              \
    template <typename left_signal_t,                                                              \
        typename right_signal_t,                                                                   \
        typename left_val_t = typename left_signal_t::value_t,                                     \
        typename right_val_t = typename right_signal_t::value_t,                                   \
        class = typename std::enable_if<is_signal<left_signal_t>::value>::type,                    \
        class = typename std::enable_if<is_signal<right_signal_t>::value>::type,                   \
        typename F = ::ureact::detail::op_functors::op_functor_##name<left_val_t, right_val_t>,    \
        typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,                  \
        typename op_t = ::ureact::detail::function_op<S,                                           \
            F,                                                                                     \
            ::ureact::detail::signal_node_ptr_t<left_val_t>,                                       \
            ::ureact::detail::signal_node_ptr_t<right_val_t>>>                                     \
    auto operator op( const left_signal_t& lhs, const right_signal_t& rhs )                        \
        -> detail::temp_signal<S, op_t>                                                            \
    {                                                                                              \
        context& context = lhs.get_context();                                                      \
        assert( context == rhs.get_context() );                                                    \
        return detail::temp_signal<S, op_t>(                                                       \
            std::make_shared<::ureact::detail::signal_op_node<S, op_t>>(                           \
                context, F(), get_node_ptr( lhs ), get_node_ptr( rhs ) ) );                        \
    }                                                                                              \
                                                                                                   \
    template <typename left_signal_t,                                                              \
        typename right_val_in_t,                                                                   \
        typename left_val_t = typename left_signal_t::value_t,                                     \
        typename right_val_t = typename std::decay<right_val_in_t>::type,                          \
        class = typename std::enable_if<is_signal<left_signal_t>::value>::type,                    \
        class = typename std::enable_if<!is_signal<right_val_t>::value>::type,                     \
        typename F = ::ureact::detail::op_functors::op_l_functor_##name<left_val_t, right_val_t>,  \
        typename S = typename std::result_of<F( left_val_t )>::type,                               \
        typename op_t                                                                              \
        = ::ureact::detail::function_op<S, F, ::ureact::detail::signal_node_ptr_t<left_val_t>>>    \
    auto operator op( const left_signal_t& lhs, right_val_in_t&& rhs )                             \
        -> detail::temp_signal<S, op_t>                                                            \
    {                                                                                              \
        context& context = lhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>(                                                       \
            std::make_shared<::ureact::detail::signal_op_node<S, op_t>>(                           \
                context, F( std::forward<right_val_in_t>( rhs ) ), get_node_ptr( lhs ) ) );        \
    }                                                                                              \
                                                                                                   \
    template <typename left_val_in_t,                                                              \
        typename right_signal_t,                                                                   \
        typename left_val_t = typename std::decay<left_val_in_t>::type,                            \
        typename right_val_t = typename right_signal_t::value_t,                                   \
        class = typename std::enable_if<!is_signal<left_val_t>::value>::type,                      \
        class = typename std::enable_if<is_signal<right_signal_t>::value>::type,                   \
        typename F = ::ureact::detail::op_functors::op_r_functor_##name<left_val_t, right_val_t>,  \
        typename S = typename std::result_of<F( right_val_t )>::type,                              \
        typename op_t                                                                              \
        = ::ureact::detail::function_op<S, F, ::ureact::detail::signal_node_ptr_t<right_val_t>>>   \
    auto operator op( left_val_in_t&& lhs, const right_signal_t& rhs )                             \
        -> detail::temp_signal<S, op_t>                                                            \
    {                                                                                              \
        context& context = rhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>( context,                                              \
            std::make_shared<::ureact::detail::signal_op_node<S, op_t>>(                           \
                context, F( std::forward<left_val_in_t>( lhs ) ), get_node_ptr( rhs ) ) );         \
    }                                                                                              \
    template <typename left_val_t,                                                                 \
        typename left_op_t,                                                                        \
        typename right_val_t,                                                                      \
        typename right_op_t,                                                                       \
        typename F = ::ureact::detail::op_functors::op_functor_##name<left_val_t, right_val_t>,    \
        typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,                  \
        typename op_t = ::ureact::detail::function_op<S, F, left_op_t, right_op_t>>                \
    auto operator op( detail::temp_signal<left_val_t, left_op_t>&& lhs,                            \
        detail::temp_signal<right_val_t, right_op_t>&& rhs ) -> detail::temp_signal<S, op_t>       \
    {                                                                                              \
        context& context = lhs.get_context();                                                      \
        assert( context == rhs.get_context() );                                                    \
        return detail::temp_signal<S, op_t>( context,                                              \
            std::make_shared<::ureact::detail::signal_op_node<S, op_t>>(                           \
                context, F(), lhs.steal_op(), rhs.steal_op() ) );                                  \
    }                                                                                              \
                                                                                                   \
    template <typename left_val_t,                                                                 \
        typename left_op_t,                                                                        \
        typename right_signal_t,                                                                   \
        typename right_val_t = typename right_signal_t::value_t,                                   \
        class = typename std::enable_if<is_signal<right_signal_t>::value>::type,                   \
        typename F = ::ureact::detail::op_functors::op_functor_##name<left_val_t, right_val_t>,    \
        typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,                  \
        typename op_t = ::ureact::detail::                                                         \
            function_op<S, F, left_op_t, ::ureact::detail::signal_node_ptr_t<right_val_t>>>        \
    auto operator op( detail::temp_signal<left_val_t, left_op_t>&& lhs,                            \
        const right_signal_t& rhs ) -> detail::temp_signal<S, op_t>                                \
    {                                                                                              \
        context& context = rhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>(                                                       \
            std::make_shared<::ureact::detail::signal_op_node<S, op_t>>(                           \
                context, F(), lhs.steal_op(), get_node_ptr( rhs ) ) );                             \
    }                                                                                              \
                                                                                                   \
    template <typename left_signal_t,                                                              \
        typename right_val_t,                                                                      \
        typename right_op_t,                                                                       \
        typename left_val_t = typename left_signal_t::value_t,                                     \
        class = typename std::enable_if<is_signal<left_signal_t>::value>::type,                    \
        typename F = ::ureact::detail::op_functors::op_functor_##name<left_val_t, right_val_t>,    \
        typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,                  \
        typename op_t = ::ureact::detail::                                                         \
            function_op<S, F, ::ureact::detail::signal_node_ptr_t<left_val_t>, right_op_t>>        \
    auto operator op( const left_signal_t& lhs,                                                    \
        detail::temp_signal<right_val_t, right_op_t>&& rhs ) -> detail::temp_signal<S, op_t>       \
    {                                                                                              \
        context& context = lhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>(                                                       \
            std::make_shared<::ureact::detail::signal_op_node<S, op_t>>(                           \
                context, F(), get_node_ptr( lhs ), rhs.steal_op() ) );                             \
    }                                                                                              \
                                                                                                   \
    template <typename left_val_t,                                                                 \
        typename left_op_t,                                                                        \
        typename right_val_in_t,                                                                   \
        typename right_val_t = typename std::decay<right_val_in_t>::type,                          \
        class = typename std::enable_if<!is_signal<right_val_t>::value>::type,                     \
        typename F = ::ureact::detail::op_functors::op_l_functor_##name<left_val_t, right_val_t>,  \
        typename S = typename std::result_of<F( left_val_t )>::type,                               \
        typename op_t = ::ureact::detail::function_op<S, F, left_op_t>>                            \
    auto operator op( detail::temp_signal<left_val_t, left_op_t>&& lhs, right_val_in_t&& rhs )     \
        -> detail::temp_signal<S, op_t>                                                            \
    {                                                                                              \
        context& context = lhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>(                                                       \
            std::make_shared<::ureact::detail::signal_op_node<S, op_t>>(                           \
                context, F( std::forward<right_val_in_t>( rhs ) ), lhs.steal_op() ) );             \
    }                                                                                              \
                                                                                                   \
    template <typename left_val_in_t,                                                              \
        typename right_val_t,                                                                      \
        typename right_op_t,                                                                       \
        typename left_val_t = typename std::decay<left_val_in_t>::type,                            \
        class = typename std::enable_if<!is_signal<left_val_t>::value>::type,                      \
        typename F = ::ureact::detail::op_functors::op_r_functor_##name<left_val_t, right_val_t>,  \
        typename S = typename std::result_of<F( right_val_t )>::type,                              \
        typename op_t = ::ureact::detail::function_op<S, F, right_op_t>>                           \
    auto operator op( left_val_in_t&& lhs, detail::temp_signal<right_val_t, right_op_t>&& rhs )    \
        -> detail::temp_signal<S, op_t>                                                            \
    {                                                                                              \
        context& context = rhs.get_context();                                                      \
        return detail::temp_signal<S, op_t>( context,                                              \
            std::make_shared<::ureact::detail::signal_op_node<S, op_t>>(                           \
                context, F( std::forward<left_val_in_t>( lhs ) ), rhs.steal_op() ) );              \
    }                                                                                              \
    }

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
