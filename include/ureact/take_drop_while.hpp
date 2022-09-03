//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_TAKE_DROP_WHILE_HPP
#define UREACT_TAKE_DROP_WHILE_HPP

#include "filter.hpp"

UREACT_BEGIN_NAMESPACE

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
UREACT_WARN_UNUSED_RESULT auto take_while(
    const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred )
{
    auto taker_while = [passed = true, pred = std::forward<Pred>( pred )](
                           const auto& e, const auto... deps ) mutable {
        passed = passed && pred( e, deps... );
        return passed;
    };

    return filter( source, dep_pack, taker_while );
}

/*!
 * @brief Curried version of take_while(const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred) algorithm used for "pipe" syntax
 */
template <typename... Deps, typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto take_while(
    const signal_pack<Deps...>& dep_pack, Pred&& pred )
{
    return closure{ [deps = dep_pack.store(), pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return take_while( std::forward<arg_t>( source ), signal_pack<Deps...>( deps ), pred );
    } };
}

/*!
 * @brief Keeps the first elements of the source stream that satisfy the unary predicate
 *
 *  Keeps events from the source stream, starting at the beginning and ending
 *  at the first element for which the predicate returns false.
 *  Semantically equivalent of std::ranges::views::take_while
 */
template <typename E, typename Pred>
UREACT_WARN_UNUSED_RESULT auto take_while( const events<E>& source, Pred&& pred )
{
    return take_while( source, signal_pack<>(), std::forward<Pred>( pred ) );
}

/*!
 * @brief Curried version of take_while(const events<E>& source, Pred&& pred) algorithm used for "pipe" syntax
 */
template <typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto take_while( Pred&& pred )
{
    return closure{ [pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return take_while( std::forward<arg_t>( source ), pred );
    } };
}

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
UREACT_WARN_UNUSED_RESULT auto drop_while(
    const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred )
{
    auto dropper_while = [passed = false, pred = std::forward<Pred>( pred )](
                             const auto& e, const auto... deps ) mutable {
        passed = passed || !pred( e, deps... );
        return passed;
    };

    return filter( source, dep_pack, dropper_while );
}

/*!
 * @brief Curried version of drop_while(const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred) algorithm used for "pipe" syntax
 */
template <typename... Deps, typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto drop_while(
    const signal_pack<Deps...>& dep_pack, Pred&& pred )
{
    return closure{ [deps = dep_pack.store(), pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return drop_while( std::forward<arg_t>( source ), signal_pack<Deps...>( deps ), pred );
    } };
}

/*!
 * @brief Skips the first elements of the source stream that satisfy the unary predicate
 *
 *  Takes events beginning at the first for which the predicate returns false.
 *  Semantically equivalent of std::ranges::views::drop_while
 */
template <typename E, typename Pred>
UREACT_WARN_UNUSED_RESULT auto drop_while( const events<E>& source, Pred&& pred )
{
    return drop_while( source, signal_pack<>(), std::forward<Pred>( pred ) );
}

/*!
 * @brief Curried version of drop_while(const events<E>& source, Pred&& pred) algorithm used for "pipe" syntax
 */
template <typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto drop_while( Pred&& pred )
{
    return closure{ [pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return drop_while( std::forward<arg_t>( source ), pred );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_TAKE_DROP_WHILE_HPP
