//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_TAP_HPP
#define UREACT_TAP_HPP

#include <ureact/observe.hpp>
#include <ureact/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Create observer for signal and return observed signal
 *
 *  When the signal value S of subject changes, func is called
 *
 *  The signature of func should be equivalent to:
 *  * void func(const S&)
 *  * observer_action func(const S&)
 *
 *  By returning observer_action::stop_and_detach, the observer function can request
 *  its own detachment. Returning observer_action::next keeps the observer attached.
 *  Using a void return type is the same as always returning observer_action::next.
 */
template <typename F,
    typename Signal, //
    class = std::enable_if_t<is_signal_v<std::decay_t<Signal>>>>
UREACT_WARN_UNUSED_RESULT auto tap( Signal&& subject, F&& func )
{
    std::ignore = observe_signal_impl( subject, std::forward<F>( func ) );
    return std::forward<Signal>( subject );
}

/*!
 * @brief Create observer for event stream and return observed event stream
 *
 *  For every event e in subject, func is called.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of func should be equivalent to:
 *  * observer_action func(event_range<E> range, const Deps& ...)
 *  * observer_action func(const E&, const Deps& ...)
 *  * void func(event_range<E> range, const Deps& ...)
 *  * void func(const E&, const Deps& ...)
 *
 *  By returning observer_action::stop_and_detach, the observer function can request
 *  its own detachment. Returning observer_action::next keeps the observer attached.
 *  Using a void return type is the same as always returning observer_action::next.
 *
 *  @note The event_range<E> option allows to explicitly batch process single turn events
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename F,
    typename Events,
    typename... Deps,
    class = std::enable_if_t<is_event_v<std::decay_t<Events>>>>
UREACT_WARN_UNUSED_RESULT auto tap(
    Events&& subject, const signal_pack<Deps...>& dep_pack, F&& func )
{
    std::ignore = observe_events_impl( subject, dep_pack, std::forward<F>( func ) );
    return std::forward<Events>( subject );
}

/*!
 * @brief Create observer for event stream and return observed event stream
 *
 *  Version without synchronization with additional signals
 *
 *  See tap(Events&& subject, const signal_pack<Deps...>& dep_pack, F&& func)
 */
template <typename F,
    typename Events,
    int wtf = 0, // hack to resolve ambiguity with signal version of tap
    class = std::enable_if_t<is_event_v<std::decay_t<Events>>>>
UREACT_WARN_UNUSED_RESULT auto tap( Events&& subject, F&& func )
{
    std::ignore = observe_events_impl( subject, signal_pack<>{}, std::forward<F>( func ) );
    return std::forward<Events>( subject );
}

/*!
 * @brief Curried version of tap(T&& subject, F&& func)
 */
template <typename F>
UREACT_WARN_UNUSED_RESULT auto tap( F&& func )
{
    return detail::closure{ [func = std::forward<F>( func )]( auto&& subject ) {
        using arg_t = decltype( subject );
        static_assert(
            is_observable_v<std::decay_t<arg_t>>, "Observable type is required (signal or event)" );
        return tap( std::forward<arg_t>( subject ), func );
    } };
}

/*!
 * @brief Curried version of tap(T&& subject, const signal_pack<Deps...>& dep_pack, F&& func)
 */
template <typename F, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto tap( const signal_pack<Deps...>& dep_pack, F&& func )
{
    return detail::closure{
        [dep_pack = dep_pack, func = std::forward<F>( func )]( auto&& subject ) {
            using arg_t = decltype( subject );
            static_assert( is_observable_v<std::decay_t<arg_t>>,
                "Observable type is required (signal or event)" );
            return tap( std::forward<arg_t>( subject ), dep_pack, func );
        } };
}

UREACT_END_NAMESPACE

#endif // UREACT_TAP_HPP
