#pragma once

#include "ureact/detail/signal.hpp"

namespace ureact
{

namespace detail
{

template <template <typename, typename> class functor_binary_op,
    typename lhs_t,
    typename rhs_t,
    typename F = functor_binary_op<lhs_t, rhs_t>>
struct bind_left
{
    bind_left( bind_left&& other ) noexcept = default;

    bind_left& operator=( bind_left&& other ) noexcept = delete;

    template <typename T,
        class = typename std::enable_if<!is_same_decay<T, bind_left>::value>::type>
    explicit bind_left( T&& val )
        : m_left_val( std::forward<T>( val ) )
    {}

    bind_left( const bind_left& other ) = delete;

    bind_left& operator=( const bind_left& other ) = delete;

    ~bind_left() = default;

    auto operator()( const rhs_t& rhs ) const
        -> decltype( std::declval<F>()( std::declval<lhs_t>(), std::declval<rhs_t>() ) )
    {
        return F()( m_left_val, rhs );
    }

    lhs_t m_left_val;
};

template <template <typename, typename> class functor_binary_op,
    typename lhs_t,
    typename rhs_t,
    typename F = functor_binary_op<lhs_t, rhs_t>>
struct bind_right
{
    bind_right( bind_right&& other ) noexcept = default;

    bind_right& operator=( bind_right&& other ) noexcept = delete;

    template <typename T,
        class = typename std::enable_if<!is_same_decay<T, bind_right>::value>::type>
    explicit bind_right( T&& val )
        : m_right_val( std::forward<T>( val ) )
    {}

    bind_right( const bind_right& other ) = delete;

    bind_right& operator=( const bind_right& other ) = delete;

    ~bind_right() = default;

    auto operator()( const lhs_t& lhs ) const
        -> decltype( std::declval<F>()( std::declval<lhs_t>(), std::declval<rhs_t>() ) )
    {
        return F()( lhs, m_right_val );
    }

    rhs_t m_right_val;
};

template <template <typename> class functor_op,
    typename signal_t,
    typename val_t = typename signal_t::value_t,
    class = typename std::enable_if<is_signal<signal_t>::value>::type,
    typename F = functor_op<val_t>,
    typename S = typename std::result_of<F( val_t )>::type,
    typename op_t = function_op<S, F, signal_node_ptr_t<val_t>>>
auto unary_operator_impl( const signal_t& arg ) -> temp_signal<S, op_t>
{
    return make_temp_signal<S, op_t>( arg.get_context(), F(), get_node_ptr( arg ) );
}

template <template <typename> class functor_op,
    typename val_t,
    typename op_in_t,
    typename F = functor_op<val_t>,
    typename S = typename std::result_of<F( val_t )>::type,
    typename op_t = function_op<S, F, op_in_t>>
auto unary_operator_impl( temp_signal<val_t, op_in_t>&& arg ) -> temp_signal<S, op_t>
{
    return make_temp_signal<S, op_t>( arg.get_context(), F(), arg.steal_op() );
}

template <template <typename, typename> class functor_op,
    typename left_signal_t,
    typename right_signal_t,
    typename left_val_t = typename left_signal_t::value_t,
    typename right_val_t = typename right_signal_t::value_t,
    class = typename std::enable_if<is_signal<left_signal_t>::value>::type,
    class = typename std::enable_if<is_signal<right_signal_t>::value>::type,
    typename F = functor_op<left_val_t, right_val_t>,
    typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,
    typename op_t = detail::function_op<S,
        F,
        detail::signal_node_ptr_t<left_val_t>,
        detail::signal_node_ptr_t<right_val_t>>>
auto binary_operator_impl( const left_signal_t& lhs, const right_signal_t& rhs )
    -> detail::temp_signal<S, op_t>
{
    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return make_temp_signal<S, op_t>( context, F(), get_node_ptr( lhs ), get_node_ptr( rhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_signal_t,
    typename right_val_in_t,
    typename left_val_t = typename left_signal_t::value_t,
    typename right_val_t = typename std::decay<right_val_in_t>::type,
    class = typename std::enable_if<is_signal<left_signal_t>::value>::type,
    class = typename std::enable_if<!is_signal<right_val_t>::value>::type,
    typename F = bind_right<functor_op, left_val_t, right_val_t>,
    typename S = typename std::result_of<F( left_val_t )>::type,
    typename op_t = detail::function_op<S, F, detail::signal_node_ptr_t<left_val_t>>>
auto binary_operator_impl( const left_signal_t& lhs, right_val_in_t&& rhs )
    -> detail::temp_signal<S, op_t>
{
    context& context = lhs.get_context();

    return make_temp_signal<S, op_t>(
        context, F( std::forward<right_val_in_t>( rhs ) ), get_node_ptr( lhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_val_in_t,
    typename right_signal_t,
    typename left_val_t = typename std::decay<left_val_in_t>::type,
    typename right_val_t = typename right_signal_t::value_t,
    class = typename std::enable_if<!is_signal<left_val_t>::value>::type,
    class = typename std::enable_if<is_signal<right_signal_t>::value>::type,
    typename F = bind_left<functor_op, left_val_t, right_val_t>,
    typename S = typename std::result_of<F( right_val_t )>::type,
    typename op_t = detail::function_op<S, F, detail::signal_node_ptr_t<right_val_t>>>
auto binary_operator_impl( left_val_in_t&& lhs, const right_signal_t& rhs )
    -> detail::temp_signal<S, op_t>
{
    context& context = rhs.get_context();

    return make_temp_signal<S, op_t>(
        context, F( std::forward<left_val_in_t>( lhs ) ), get_node_ptr( rhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_val_t,
    typename left_op_t,
    typename right_val_t,
    typename right_op_t,
    typename F = functor_op<left_val_t, right_val_t>,
    typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,
    typename op_t = detail::function_op<S, F, left_op_t, right_op_t>>
auto binary_operator_impl( detail::temp_signal<left_val_t, left_op_t>&& lhs,
    detail::temp_signal<right_val_t, right_op_t>&& rhs ) -> detail::temp_signal<S, op_t>
{
    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return make_temp_signal<S, op_t>( context, F(), lhs.steal_op(), rhs.steal_op() );
}

template <template <typename, typename> class functor_op,
    typename left_val_t,
    typename left_op_t,
    typename right_signal_t,
    typename right_val_t = typename right_signal_t::value_t,
    class = typename std::enable_if<is_signal<right_signal_t>::value>::type,
    typename F = functor_op<left_val_t, right_val_t>,
    typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,
    typename op_t = detail::function_op<S, F, left_op_t, detail::signal_node_ptr_t<right_val_t>>>
auto binary_operator_impl( detail::temp_signal<left_val_t, left_op_t>&& lhs,
    const right_signal_t& rhs ) -> detail::temp_signal<S, op_t>
{
    context& context = rhs.get_context();

    return make_temp_signal<S, op_t>( context, F(), lhs.steal_op(), get_node_ptr( rhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_signal_t,
    typename right_val_t,
    typename right_op_t,
    typename left_val_t = typename left_signal_t::value_t,
    class = typename std::enable_if<is_signal<left_signal_t>::value>::type,
    typename F = functor_op<left_val_t, right_val_t>,
    typename S = typename std::result_of<F( left_val_t, right_val_t )>::type,
    typename op_t = detail::function_op<S, F, detail::signal_node_ptr_t<left_val_t>, right_op_t>>
auto binary_operator_impl( const left_signal_t& lhs,
    detail::temp_signal<right_val_t, right_op_t>&& rhs ) -> detail::temp_signal<S, op_t>
{
    context& context = lhs.get_context();

    return make_temp_signal<S, op_t>( context, F(), get_node_ptr( lhs ), rhs.steal_op() );
}

template <template <typename, typename> class functor_op,
    typename left_val_t,
    typename left_op_t,
    typename right_val_in_t,
    typename right_val_t = typename std::decay<right_val_in_t>::type,
    class = typename std::enable_if<!is_signal<right_val_t>::value>::type,
    typename F = bind_right<functor_op, left_val_t, right_val_t>,
    typename S = typename std::result_of<F( left_val_t )>::type,
    typename op_t = detail::function_op<S, F, left_op_t>>
auto binary_operator_impl( detail::temp_signal<left_val_t, left_op_t>&& lhs, right_val_in_t&& rhs )
    -> detail::temp_signal<S, op_t>
{
    context& context = lhs.get_context();

    return make_temp_signal<S, op_t>(
        context, F( std::forward<right_val_in_t>( rhs ) ), lhs.steal_op() );
}

template <template <typename, typename> class functor_op,
    typename left_val_in_t,
    typename right_val_t,
    typename right_op_t,
    typename left_val_t = typename std::decay<left_val_in_t>::type,
    class = typename std::enable_if<!is_signal<left_val_t>::value>::type,
    typename F = bind_left<functor_op, left_val_t, right_val_t>,
    typename S = typename std::result_of<F( right_val_t )>::type,
    typename op_t = detail::function_op<S, F, right_op_t>>
auto binary_operator_impl( left_val_in_t&& lhs, detail::temp_signal<right_val_t, right_op_t>&& rhs )
    -> detail::temp_signal<S, op_t>
{
    context& context = rhs.get_context();

    return make_temp_signal<S, op_t>(
        context, F( std::forward<left_val_in_t>( lhs ) ), rhs.steal_op() );
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

#define UREACT_DECLARE_UNARY_OP( op, name )                                                        \
    template <typename arg_t,                                                                      \
        template <typename> class functor_op = detail::op_functors::op_functor_##name>             \
    auto operator op( arg_t&& arg )                                                                \
        ->decltype( detail::unary_operator_impl<functor_op>( std::forward<arg_t>( arg ) ) )        \
    {                                                                                              \
        return detail::unary_operator_impl<functor_op>( std::forward<arg_t&&>( arg ) );            \
    }

#define UREACT_DECLARE_BINARY_OP( op, name )                                                       \
    template <typename lhs_t,                                                                      \
        typename rhs_t,                                                                            \
        template <typename, typename> class functor_op = detail::op_functors::op_functor_##name>   \
    auto operator op( lhs_t&& lhs, rhs_t&& rhs )                                                   \
        ->decltype( detail::binary_operator_impl<functor_op>(                                      \
            std::forward<lhs_t&&>( lhs ), std::forward<rhs_t&&>( rhs ) ) )                         \
    {                                                                                              \
        return detail::binary_operator_impl<functor_op>(                                           \
            std::forward<lhs_t&&>( lhs ), std::forward<rhs_t&&>( rhs ) );                          \
    }

#define UREACT_DECLARE_UNARY_OPERATOR( op, name )                                                  \
    UREACT_DECLARE_UNARY_OP_FUNCTOR( op, name )                                                    \
    UREACT_DECLARE_UNARY_OP( op, name )

#define UREACT_DECLARE_BINARY_OPERATOR( op, name )                                                 \
    UREACT_DECLARE_BINARY_OP_FUNCTOR( op, name )                                                   \
    UREACT_DECLARE_BINARY_OP( op, name )

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wunknown-warning-option"
#    pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#endif

UREACT_DECLARE_UNARY_OPERATOR( +, unary_plus )
UREACT_DECLARE_UNARY_OPERATOR( -, unary_minus )
UREACT_DECLARE_UNARY_OPERATOR( !, logical_negation )
UREACT_DECLARE_UNARY_OPERATOR( ~, bitwise_complement )

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

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic pop
#endif

#undef UREACT_DECLARE_UNARY_OPERATOR
#undef UREACT_DECLARE_UNARY_OP_FUNCTOR
#undef UREACT_DECLARE_UNARY_OP
#undef UREACT_DECLARE_BINARY_OPERATOR
#undef UREACT_DECLARE_BINARY_OP_FUNCTOR
#undef UREACT_DECLARE_BINARY_OP

} // namespace ureact
