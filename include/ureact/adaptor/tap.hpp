//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_TAP_HPP
#define UREACT_ADAPTOR_TAP_HPP

#include <ureact/adaptor/observe.hpp>
#include <ureact/utility/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct TapAdaptor : Adaptor
{
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
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Signal&& subject, F&& func ) const
    {
        std::ignore = observe( subject, std::forward<F>( func ) );
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
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        Events&& subject, const signal_pack<Deps...>& dep_pack, F&& func ) const
    {
        std::ignore = observe( subject, dep_pack, std::forward<F>( func ) );
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
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Events&& subject, F&& func ) const
    {
        return operator()( subject, signal_pack<>{}, std::forward<F>( func ) );
    }

    /*!
	 * @brief Curried version of tap(T&& subject, F&& func)
	 */
    template <typename F>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( F&& func ) const
    {
        return make_partial<TapAdaptor>( std::forward<F>( func ) );
    }

    /*!
	 * @brief Curried version of tap(T&& subject, const signal_pack<Deps...>& dep_pack, F&& func)
	 */
    template <typename F, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const signal_pack<Deps...>& dep_pack, F&& func ) const
    {
        return make_partial<TapAdaptor>( dep_pack, std::forward<F>( func ) );
    }
};

} // namespace detail

inline constexpr detail::TapAdaptor tap;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_TAP_HPP
