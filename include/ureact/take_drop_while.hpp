//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_TAKE_DROP_WHILE_HPP
#define UREACT_TAKE_DROP_WHILE_HPP

#include <ureact/detail/closure.hpp>
#include <ureact/filter.hpp>
#include <ureact/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename Derived>
struct TakeDropWhileAdaptorBase : Adaptor
{
    template <typename E, typename Pred>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<E>& source, Pred&& pred ) const
    {
        return Derived{}( source, signal_pack<>(), std::forward<Pred>( pred ) );
    }

    template <typename... Deps, typename Pred>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const signal_pack<Deps...>& dep_pack, Pred&& pred ) const
    {
        return make_partial<Derived>( dep_pack, std::forward<Pred>( pred ) );
    }

    template <typename Pred>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Pred&& pred ) const
    {
        return make_partial<Derived>( std::forward<Pred>( pred ) );
    }
};

struct TakeWhileAdaptor : TakeDropWhileAdaptorBase<TakeWhileAdaptor>
{

/*!
 * @brief Keeps the first elements of the source stream that satisfy the predicate
 *
 *  Keeps events from the source stream, starting at the beginning and ending
 *  at the first element for which the predicate returns false.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of pred should be equivalent to:
 *  * bool func(const E&, const Deps& ...)
 */
template <typename E, typename... Deps, typename Pred>
UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
    const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred ) const
{
    return filter( source,
        dep_pack,
        [passed = true, pred = std::forward<Pred>( pred )] //
        ( const auto& e, const auto... deps ) mutable {
            passed = passed && std::invoke( pred, e, deps... );
            return passed;
        } );
}

    using TakeDropWhileAdaptorBase::operator();
};

struct DropWhileAdaptor : TakeDropWhileAdaptorBase<DropWhileAdaptor>
{

/*!
 * @brief Skips the first elements of the source stream that satisfy the predicate
 *
 *  Takes events beginning at the first for which the predicate returns false.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of pred should be equivalent to:
 *  * bool func(const E&, const Deps& ...)
 */
template <typename E, typename... Deps, typename Pred>
UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
    const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred ) const
{
    return filter( source,
        dep_pack,
        [passed = false, pred = std::forward<Pred>( pred )] //
        ( const auto& e, const auto... deps ) mutable {
            passed = passed || !std::invoke( pred, e, deps... );
            return passed;
        } );
}

    using TakeDropWhileAdaptorBase::operator();
};

} // namespace detail

/*!
 * @brief Keeps the first elements of the source stream that satisfy the predicate
 *
 *  Keeps events from the source stream, starting at the beginning and ending
 *  at the first element for which the predicate returns false.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of pred should be equivalent to:
 *  * bool func(const E&, const Deps& ...)
 */
inline constexpr detail::TakeWhileAdaptor take_while;

/*!
 * @brief Skips the first elements of the source stream that satisfy the predicate
 *
 *  Takes events beginning at the first for which the predicate returns false.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of pred should be equivalent to:
 *  * bool func(const E&, const Deps& ...)
 */
inline constexpr detail::DropWhileAdaptor drop_while;

UREACT_END_NAMESPACE

#endif // UREACT_TAKE_DROP_WHILE_HPP
