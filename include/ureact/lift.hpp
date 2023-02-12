//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_LIFT_HPP
#define UREACT_LIFT_HPP

#include <ureact/detail/base.hpp>
#include <ureact/detail/closure.hpp>
#include <ureact/detail/reactive_op_base.hpp>
#include <ureact/signal.hpp>
#include <ureact/signal_pack.hpp>
#include <ureact/temp_signal.hpp>
#include <ureact/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
using signal_node_ptr_t = std::shared_ptr<signal_node<S>>;

template <typename S, typename F, typename... Deps>
class function_op : public reactive_op_base<Deps...>
{
public:
    template <typename InF, typename... Args>
    explicit function_op( InF&& func, Args&&... args )
        : function_op::reactive_op_base( dont_move(), std::forward<Args>( args )... )
        , m_func( std::forward<InF>( func ) )
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
            return std::invoke( m_func, eval( std::forward<T>( args ) )... );
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

// TODO: remove r-value reference only, leave simple references
template <typename SIn, typename F, typename... Values>
using deduce_s = std::conditional_t<std::is_same_v<SIn, void>, //
    std::decay_t<std::invoke_result_t<F, Values...>>,
    SIn>;

// Based on the transparent functor std::negate<>
struct unary_plus
{
    template <typename T>
    constexpr auto operator()( T&& t ) const          //
        noexcept( noexcept( +std::forward<T>( t ) ) ) //
        -> decltype( +std::forward<T>( t ) )
    {
        return +std::forward<T>( t );
    }

    using is_transparent = void;
};

template <typename SIn = void>
struct LiftAdaptor : Adaptor
{

    /*!
	 * @brief Create a new signal node with value v = std::invoke(func, arg_pack.get(), ...)
	 * @tparam Values types of signal values
	 *
	 * This value is set on construction and updated when any args have changed
	 */
    template <typename... Values, typename InF>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const signal_pack<Values...>& arg_pack, InF&& func ) const
    {
        using F = std::decay_t<InF>;
        using S = detail::deduce_s<SIn, F, Values...>;
        using Op = detail::function_op<S, F, detail::signal_node_ptr_t<Values>...>;

        context& context = std::get<0>( arg_pack.data ).get_context();

        auto node_builder = [&context, &func]( const signal<Values>&... args ) {
            return temp_signal<S, Op>{ context, std::forward<InF>( func ), args.get_node()... };
        };

        return std::apply( node_builder, arg_pack.data );
    }

    /*!
	 * @brief Create a new signal node with value v = std::invoke(func, arg.get())
	 *
	 * This value is set on construction and updated when arg have changed
	 */
    template <typename Value, typename InF>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const signal<Value>& arg, InF&& func ) const
    {
        using F = std::decay_t<InF>;
        using S = detail::deduce_s<SIn, F, Value>;
        using Op = detail::function_op<S, F, detail::signal_node_ptr_t<Value>>;
        return temp_signal<S, Op>{ arg.get_context(), std::forward<InF>( func ), arg.get_node() };
    }

    /*!
	 * @brief Create a new signal node with value v = std::invoke(func, arg.get())
	 *
	 * This value is set on construction and updated when arg have changed
	 */
    template <typename Value, typename OpIn, typename InF>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        temp_signal<Value, OpIn>&& arg, InF&& func ) const
    {
        using F = std::decay_t<InF>;
        using S = detail::deduce_s<SIn, F, Value>;
        using Op = detail::function_op<S, F, OpIn>;
        return temp_signal<S, Op>{
            arg.get_context(), std::forward<InF>( func ), std::move( arg ).steal_op() };
    }

    /*!
	 * @brief Curried version of lift(const signal_pack<Values...>& arg_pack, InF&& func)
	 */
    template <typename InF>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( InF&& func ) const
    {
        return make_partial<LiftAdaptor>( std::forward<InF>( func ) );
    }

    /*!
	 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
	 *
	 * This value is set on construction and updated when arg have changed
	 */
    template <typename LeftSignal,
        typename InF,
        typename RightSignal,
        class = std::enable_if_t<is_signal_v<LeftSignal>>,
        class = std::enable_if_t<is_signal_v<RightSignal>>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const LeftSignal& lhs, InF&& func, const RightSignal& rhs ) const
    {
        return operator()( with( lhs, rhs ), std::forward<InF>( func ) );
    }

    /*!
	 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
	 *
	 * This value is set on construction and updated when arg have changed
	 */
    template <typename LeftSignal,
        typename InF,
        typename RightVal,
        class = std::enable_if_t<is_signal_v<std::decay_t<LeftSignal>>>,
        class = std::enable_if_t<!is_signal_v<std::decay_t<RightVal>>>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        LeftSignal&& lhs, InF&& func, RightVal&& rhs ) const
    {
        return operator()( std::forward<LeftSignal>( lhs ),
            std::bind(
                std::forward<InF>( func ), std::placeholders::_1, std::forward<RightVal>( rhs ) ) );
    }

    /*!
	 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
	 *
	 * This value is set on construction and updated when arg have changed
	 */
    template <typename LeftVal,
        typename InF,
        typename RightSignal,
        class = std::enable_if_t<!is_signal_v<std::decay_t<LeftVal>>>,
        class = std::enable_if_t<is_signal_v<std::decay_t<RightSignal>>>,
        typename Wtf = void>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        LeftVal&& lhs, InF&& func, RightSignal&& rhs ) const
    {
        return operator()( std::forward<RightSignal>( rhs ),
            std::bind(
                std::forward<InF>( func ), std::forward<LeftVal>( lhs ), std::placeholders::_1 ) );
    }

    /*!
	 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
	 *
	 * This value is set on construction and updated when arg have changed
	 */
    template <typename LeftVal, typename LeftOp, typename InF, typename RightVal, typename RightOp>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        temp_signal<LeftVal, LeftOp>&& lhs, InF&& func, temp_signal<RightVal, RightOp>&& rhs ) const
    {
        using F = std::decay_t<InF>;
        using S = detail::deduce_s<SIn, F, LeftVal, RightVal>;
        using Op = detail::function_op<S, F, LeftOp, RightOp>;

        context& context = lhs.get_context();
        assert( context == rhs.get_context() );

        return temp_signal<S, Op>{ context,
            std::forward<InF>( func ),
            std::move( lhs ).steal_op(),
            std::move( rhs ).steal_op() };
    }

    /*!
	 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
	 *
	 * This value is set on construction and updated when arg have changed
	 */
    template <typename LeftVal,
        typename LeftOp,
        typename InF,
        typename RightSignal,
        class = std::enable_if_t<is_signal_v<RightSignal>>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        temp_signal<LeftVal, LeftOp>&& lhs, InF&& func, const RightSignal& rhs ) const
    {
        using RightVal = typename RightSignal::value_t;
        using F = std::decay_t<InF>;
        using S = detail::deduce_s<SIn, F, LeftVal, RightVal>;
        using Op = detail::function_op<S, F, LeftOp, detail::signal_node_ptr_t<RightVal>>;

        context& context = lhs.get_context();
        assert( context == rhs.get_context() );

        return temp_signal<S, Op>{
            context, std::forward<InF>( func ), std::move( lhs ).steal_op(), rhs.get_node() };
    }

    /*!
	 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
	 *
	 * This value is set on construction and updated when arg have changed
	 */
    template <typename LeftSignal,
        typename InF,
        typename RightVal,
        typename RightOp,
        class = std::enable_if_t<is_signal_v<LeftSignal>>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const LeftSignal& lhs, InF&& func, temp_signal<RightVal, RightOp>&& rhs ) const
    {
        using LeftVal = typename LeftSignal::value_t;
        using F = std::decay_t<InF>;
        using S = detail::deduce_s<SIn, F, LeftVal, RightVal>;
        using Op = detail::function_op<S, F, detail::signal_node_ptr_t<LeftVal>, RightOp>;

        context& context = lhs.get_context();
        assert( context == rhs.get_context() );

        return temp_signal<S, Op>{
            context, std::forward<InF>( func ), lhs.get_node(), std::move( rhs ).steal_op() };
    }
};

} // namespace detail

/*!
 * @brief Create a new signal applying function to given signals
 *
 *  Type of resulting signal should be explicitly specified.
 */
template <typename SIn = void>
inline constexpr detail::LiftAdaptor<SIn> lift_;

/*!
 * @brief Create a new signal applying function to given signals
 *
 *  Type of resulting signal should be explicitly specified.
 */
inline constexpr detail::LiftAdaptor<> lift;

#define UREACT_DECLARE_UNARY_LIFT_OPERATOR( op, fn )                                               \
    template <typename Signal, class = std::enable_if_t<is_signal_v<std::decay_t<Signal>>>>        \
    UREACT_WARN_UNUSED_RESULT auto operator op( Signal&& arg )                                     \
    {                                                                                              \
        return lift( std::forward<Signal>( arg ), fn{} );                                          \
    }

#define UREACT_DECLARE_BINARY_LIFT_OPERATOR( op, fn )                                              \
    template <typename Lhs,                                                                        \
        typename Rhs,                                                                              \
        class = std::enable_if_t<                                                                  \
            std::disjunction_v<is_signal<std::decay_t<Lhs>>, is_signal<std::decay_t<Rhs>>>>>       \
    UREACT_WARN_UNUSED_RESULT auto operator op( Lhs&& lhs, Rhs&& rhs )                             \
    {                                                                                              \
        return lift( std::forward<Lhs>( lhs ), fn{}, std::forward<Rhs>( rhs ) );                   \
    }

// arithmetic operators

UREACT_DECLARE_BINARY_LIFT_OPERATOR( +, std::plus<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( -, std::minus<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( *, std::multiplies<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( /, std::divides<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( %, std::modulus<> )
UREACT_DECLARE_UNARY_LIFT_OPERATOR( +, detail::unary_plus )
UREACT_DECLARE_UNARY_LIFT_OPERATOR( -, std::negate<> )

// relational operators

UREACT_DECLARE_BINARY_LIFT_OPERATOR( ==, std::equal_to<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( !=, std::not_equal_to<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( <, std::less<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( <=, std::less_equal<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( >, std::greater<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( >=, std::greater_equal<> )

// logical operators

UREACT_DECLARE_BINARY_LIFT_OPERATOR( &&, std::logical_and<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( ||, std::logical_or<> )
UREACT_DECLARE_UNARY_LIFT_OPERATOR( !, std::logical_not<> )

#undef UREACT_DECLARE_UNARY_LIFT_OPERATOR
#undef UREACT_DECLARE_BINARY_LIFT_OPERATOR

UREACT_END_NAMESPACE

#endif // UREACT_LIFT_HPP
