//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_LIFT_HPP
#define UREACT_LIFT_HPP

#include "ureact.hpp"

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
using signal_node_ptr_t = std::shared_ptr<signal_node<S>>;

template <typename S, typename F, typename... Deps>
class function_op : public reactive_op_base<Deps...>
{
public:
    template <typename FIn, typename... Args>
    explicit function_op( FIn&& func, Args&&... args )
        : function_op::reactive_op_base( dont_move(), std::forward<Args>( args )... )
        , m_func( std::forward<FIn>( func ) )
    {}

    function_op( function_op&& ) noexcept = default;
    function_op& operator=( function_op&& ) noexcept = default;

    UREACT_WARN_UNUSED_RESULT S evaluate()
    {
        return std::apply( eval_functor( m_func ), this->m_deps );
    }

private:
    struct eval_functor
    {
        explicit eval_functor( F& f )
            : m_func( f )
        {}

        template <typename... T>
        UREACT_WARN_UNUSED_RESULT S operator()( T&&... args )
        {
            return m_func( eval( std::forward<T>( args ) )... );
        }

        template <typename T>
        UREACT_WARN_UNUSED_RESULT static auto eval( T& op ) -> decltype( op.evaluate() )
        {
            return op.evaluate();
        }

        template <typename T>
        UREACT_WARN_UNUSED_RESULT static auto eval( const std::shared_ptr<T>& dep_ptr )
            -> decltype( dep_ptr->value_ref() )
        {
            return dep_ptr->value_ref();
        }

        F& m_func;
    };

    F m_func;
};

} // namespace detail

/*!
 * @brief Create a new signal node with value v = func(arg_pack.get(), ...).
 * @tparam Values types of signal values
 *
 * This value is set on construction and updated when any args have changed
 *
 *  The signature of func should be equivalent to:
 *  * S func(const Values& ...)
 */
template <typename... Values,
    typename InF,
    typename F = std::decay_t<InF>,
    typename S = std::invoke_result_t<F, Values...>,
    typename Op = detail::function_op<S, F, detail::signal_node_ptr_t<Values>...>>
UREACT_WARN_UNUSED_RESULT auto lift( const signal_pack<Values...>& arg_pack, InF&& func )
{
    context& context = std::get<0>( arg_pack.data ).get_context();

    auto node_builder = [&context, &func]( const signal<Values>&... args ) {
        return temp_signal<S, Op>{ context, std::forward<InF>( func ), args.get_node()... };
    };

    return std::apply( node_builder, arg_pack.data );
}

/*!
 * @brief Create a new signal node with value v = func(arg.get()).
 * This value is set on construction and updated when arg have changed
 *
 *  The signature of func should be equivalent to:
 *  * S func(const value_t&)
 */
template <typename Value, typename InF>
UREACT_WARN_UNUSED_RESULT auto lift( const signal<Value>& arg, InF&& func )
{
    return lift( with( arg ), std::forward<InF>( func ) );
}

/*!
 * @brief Curried version of lift(const signal_pack<Values...>& arg_pack, InF&& func) algorithm used for "pipe" syntax
 */
template <typename InF>
UREACT_WARN_UNUSED_RESULT auto lift( InF&& func )
{
    return closure{ [func = std::forward<InF>( func )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert(
            std::disjunction_v<is_signal<std::decay_t<arg_t>>, is_signal_pack<std::decay_t<arg_t>>>,
            "Signal type or signal_pack is required" );
        return lift( std::forward<arg_t>( source ), func );
    } };
}

namespace detail
{

// Full analog of std::binder1st that removed in c++17
// See https://en.cppreference.com/w/cpp/utility/functional/binder12
template <template <typename, typename> class FunctorBinaryOp,
    typename FirstArgument,
    typename SecondArgument,
    typename Fn = FunctorBinaryOp<FirstArgument, SecondArgument>>
class binder1st
{
public:
    template <typename T, class = disable_if_same_t<T, binder1st>>
    explicit binder1st( T&& first_argument )
        : m_first_argument( std::forward<T>( first_argument ) )
    {}

    template <typename T>
    UREACT_WARN_UNUSED_RESULT auto operator()( T&& second_argument ) const
    {
        return m_fn( m_first_argument, std::forward<T>( second_argument ) );
    }

private:
    Fn m_fn{};
    FirstArgument m_first_argument;
};

// Full analog of std::binder2nd that removed in c++17
// See https://en.cppreference.com/w/cpp/utility/functional/binder12
template <template <typename, typename> class FunctorBinaryOp,
    typename FirstArgument,
    typename SecondArgument,
    typename Fn = FunctorBinaryOp<FirstArgument, SecondArgument>>
class binder2nd
{
public:
    template <typename T, class = disable_if_same_t<T, binder2nd>>
    explicit binder2nd( T&& second_argument )
        : m_second_argument( std::forward<T>( second_argument ) )
    {}

    template <typename T>
    UREACT_WARN_UNUSED_RESULT auto operator()( T&& first_argument ) const
    {
        return m_fn( std::forward<T>( first_argument ), m_second_argument );
    }

private:
    Fn m_fn{};
    SecondArgument m_second_argument;
};

template <template <typename> class FunctorOp,
    typename Signal,
    class = std::enable_if_t<is_signal_v<Signal>>>
auto unary_operator_impl( const Signal& arg )
{
    using val_t = typename Signal::value_t;
    using F = FunctorOp<val_t>;
    using S = std::invoke_result_t<F, val_t>;
    using Op = function_op<S, F, signal_node_ptr_t<val_t>>;
    return temp_signal<S, Op>{ arg.get_context(), F(), arg.get_node() };
}

template <template <typename> class FunctorOp, typename Val, typename OpIn>
auto unary_operator_impl( temp_signal<Val, OpIn>&& arg )
{
    using F = FunctorOp<Val>;
    using S = std::invoke_result_t<F, Val>;
    using Op = function_op<S, F, OpIn>;
    return temp_signal<S, Op>{ arg.get_context(), F(), arg.steal_op() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftSignal,
    typename RightSignal,
    class = std::enable_if_t<is_signal_v<LeftSignal>>,
    class = std::enable_if_t<is_signal_v<RightSignal>>>
auto binary_operator_impl( const LeftSignal& lhs, const RightSignal& rhs )
{
    using LeftVal = typename LeftSignal::value_t;
    using RightVal = typename RightSignal::value_t;
    using F = FunctorOp<LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal, RightVal>;
    using Op = function_op<S, F, signal_node_ptr_t<LeftVal>, signal_node_ptr_t<RightVal>>;

    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return temp_signal<S, Op>{ context, F(), lhs.get_node(), rhs.get_node() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftSignal,
    typename RightValIn,
    typename LeftVal = typename LeftSignal::value_t,
    typename RightVal = std::decay_t<RightValIn>,
    class = std::enable_if_t<is_signal_v<LeftSignal>>,
    class = std::enable_if_t<!is_signal_v<RightVal>>>
auto binary_operator_impl( const LeftSignal& lhs, RightValIn&& rhs )
{
    using F = binder2nd<FunctorOp, LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal>;
    using Op = function_op<S, F, signal_node_ptr_t<LeftVal>>;

    context& context = lhs.get_context();

    return temp_signal<S, Op>{ context, F( std::forward<RightValIn>( rhs ) ), lhs.get_node() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftValIn,
    typename RightSignal,
    typename LeftVal = std::decay_t<LeftValIn>,
    typename RightVal = typename RightSignal::value_t,
    class = std::enable_if_t<!is_signal_v<LeftVal>>,
    class = std::enable_if_t<is_signal_v<RightSignal>>>
auto binary_operator_impl( LeftValIn&& lhs, const RightSignal& rhs )
{
    using F = binder1st<FunctorOp, LeftVal, RightVal>;
    using S = std::invoke_result_t<F, RightVal>;
    using Op = function_op<S, F, signal_node_ptr_t<RightVal>>;

    context& context = rhs.get_context();

    return temp_signal<S, Op>{ context, F( std::forward<LeftValIn>( lhs ) ), rhs.get_node() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftVal,
    typename LeftOp,
    typename RightVal,
    typename RightOp>
auto binary_operator_impl(
    temp_signal<LeftVal, LeftOp>&& lhs, temp_signal<RightVal, RightOp>&& rhs )
{
    using F = FunctorOp<LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal, RightVal>;
    using Op = function_op<S, F, LeftOp, RightOp>;

    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return temp_signal<S, Op>{ context, F(), lhs.steal_op(), rhs.steal_op() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftVal,
    typename LeftOp,
    typename RightSignal,
    class = std::enable_if_t<is_signal_v<RightSignal>>>
auto binary_operator_impl( temp_signal<LeftVal, LeftOp>&& lhs, const RightSignal& rhs )
{
    using RightVal = typename RightSignal::value_t;
    using F = FunctorOp<LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal, RightVal>;
    using Op = function_op<S, F, LeftOp, signal_node_ptr_t<RightVal>>;

    context& context = rhs.get_context();

    return temp_signal<S, Op>{ context, F(), lhs.steal_op(), rhs.get_node() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftSignal,
    typename RightVal,
    typename RightOp,
    class = std::enable_if_t<is_signal_v<LeftSignal>>>
auto binary_operator_impl( const LeftSignal& lhs, temp_signal<RightVal, RightOp>&& rhs )
{
    using LeftVal = typename LeftSignal::value_t;
    using F = FunctorOp<LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal, RightVal>;
    using Op = function_op<S, F, signal_node_ptr_t<LeftVal>, RightOp>;

    context& context = lhs.get_context();

    return temp_signal<S, Op>{ context, F(), lhs.get_node(), rhs.steal_op() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftVal,
    typename LeftOp,
    typename RightValIn,
    typename RightVal = std::decay_t<RightValIn>,
    class = std::enable_if_t<!is_signal_v<RightVal>>>
auto binary_operator_impl( temp_signal<LeftVal, LeftOp>&& lhs, RightValIn&& rhs )
{
    using F = binder2nd<FunctorOp, LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal>;
    using Op = function_op<S, F, LeftOp>;

    context& context = lhs.get_context();

    return temp_signal<S, Op>{ context, F( std::forward<RightValIn>( rhs ) ), lhs.steal_op() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftValIn,
    typename RightVal,
    typename RightOp,
    typename LeftVal = std::decay_t<LeftValIn>,
    class = std::enable_if_t<!is_signal_v<LeftVal>>>
auto binary_operator_impl( LeftValIn&& lhs, temp_signal<RightVal, RightOp>&& rhs )
{
    using F = binder1st<FunctorOp, LeftVal, RightVal>;
    using S = std::invoke_result_t<F, RightVal>;
    using Op = function_op<S, F, RightOp>;

    context& context = rhs.get_context();

    return temp_signal<S, Op>{ context, F( std::forward<LeftValIn>( lhs ) ), rhs.steal_op() };
}

} // namespace detail

// Define function body using "Please repeat yourself twice" idiom
#define UREACT_FUNCTION_BODY( EXPR )                                                               \
    noexcept( noexcept( EXPR ) )->decltype( EXPR )                                                 \
    {                                                                                              \
        return ( EXPR );                                                                           \
    }

// Forward arg
#define UREACT_FWD( X ) std::forward<decltype( X )>( X )

#define UREACT_DECLARE_UNARY_OP_FUNCTOR( op, name )                                                \
    namespace detail                                                                               \
    {                                                                                              \
    template <typename V>                                                                          \
    struct op_functor_##name                                                                       \
    {                                                                                              \
        UREACT_WARN_UNUSED_RESULT auto operator()( const V& v ) const UREACT_FUNCTION_BODY( op v ) \
    };                                                                                             \
    } /* namespace detail */

#define UREACT_DECLARE_UNARY_OP( op, name )                                                        \
    template <typename Signal,                                                                     \
        template <typename> class FunctorOp = detail::op_functor_##name,                           \
        class = std::enable_if_t<is_signal_v<std::decay_t<Signal>>>>                               \
    UREACT_WARN_UNUSED_RESULT auto operator op( Signal&& arg )                                     \
        UREACT_FUNCTION_BODY( detail::unary_operator_impl<FunctorOp>( UREACT_FWD( arg ) ) )

#define UREACT_DECLARE_UNARY_OPERATOR( op, name )                                                  \
    UREACT_DECLARE_UNARY_OP_FUNCTOR( op, name )                                                    \
    UREACT_DECLARE_UNARY_OP( op, name )

#define UREACT_DECLARE_BINARY_OP_FUNCTOR( op, name )                                               \
    namespace detail                                                                               \
    {                                                                                              \
    template <typename L, typename R>                                                              \
    struct op_functor_##name                                                                       \
    {                                                                                              \
        UREACT_WARN_UNUSED_RESULT auto operator()( const L& lhs, const R& rhs ) const              \
            UREACT_FUNCTION_BODY( lhs op rhs )                                                     \
    };                                                                                             \
    } /* namespace detail */

#define UREACT_DECLARE_BINARY_OP( op, name )                                                       \
    template <typename Lhs,                                                                        \
        typename Rhs,                                                                              \
        template <typename, typename> class FunctorOp = detail::op_functor_##name,                 \
        class = std::enable_if_t<                                                                  \
            std::disjunction_v<is_signal<std::decay_t<Lhs>>, is_signal<std::decay_t<Rhs>>>>>       \
    UREACT_WARN_UNUSED_RESULT auto operator op( Lhs&& lhs, Rhs&& rhs ) /*                */        \
        UREACT_FUNCTION_BODY(                                                                      \
            detail::binary_operator_impl<FunctorOp>( UREACT_FWD( lhs ), UREACT_FWD( rhs ) ) )

#define UREACT_DECLARE_BINARY_OPERATOR( op, name )                                                 \
    UREACT_DECLARE_BINARY_OP_FUNCTOR( op, name )                                                   \
    UREACT_DECLARE_BINARY_OP( op, name )

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wunknown-warning-option"
#    pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#endif

// arithmetic operators

UREACT_DECLARE_BINARY_OPERATOR( +, addition )
UREACT_DECLARE_BINARY_OPERATOR( -, subtraction )
UREACT_DECLARE_BINARY_OPERATOR( *, multiplication )
UREACT_DECLARE_BINARY_OPERATOR( /, division )
UREACT_DECLARE_BINARY_OPERATOR( %, modulo )
UREACT_DECLARE_UNARY_OPERATOR( +, unary_plus )
UREACT_DECLARE_UNARY_OPERATOR( -, unary_minus )

// relational operators

UREACT_DECLARE_BINARY_OPERATOR( ==, equal )
UREACT_DECLARE_BINARY_OPERATOR( !=, not_equal )
UREACT_DECLARE_BINARY_OPERATOR( <, less )
UREACT_DECLARE_BINARY_OPERATOR( <=, less_equal )
UREACT_DECLARE_BINARY_OPERATOR( >, greater )
UREACT_DECLARE_BINARY_OPERATOR( >=, greater_equal )

// logical operators

UREACT_DECLARE_BINARY_OPERATOR( &&, logical_and )
UREACT_DECLARE_BINARY_OPERATOR( ||, logical_or )
UREACT_DECLARE_UNARY_OPERATOR( !, logical_negation )

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic pop
#endif

#undef UREACT_DECLARE_UNARY_OPERATOR
#undef UREACT_DECLARE_UNARY_OP_FUNCTOR
#undef UREACT_DECLARE_UNARY_OP
#undef UREACT_DECLARE_BINARY_OPERATOR
#undef UREACT_DECLARE_BINARY_OP_FUNCTOR
#undef UREACT_DECLARE_BINARY_OP

UREACT_END_NAMESPACE

#endif // UREACT_LIFT_HPP
