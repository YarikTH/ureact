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
#include <ureact/utility/observe_policy.hpp>
#include <ureact/utility/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

/// Passes target signal changes and holds its observer
template <typename S>
class signal_tap_node final : public signal_node<S>
{
public:
    signal_tap_node( const context& context, const signal<S>& target, observer observer )
        : signal_tap_node::signal_node( context, get_internals( target ).value_ref() )
        , m_target( target )
        , m_observer( std::move( observer ) )
    {
        this->attach_to( m_target );
    }

    ~signal_tap_node() override
    {
        this->detach_from_all();
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        return this->try_change_value( get_internals( m_target ).get_value() );
    }

private:
    signal<S> m_target;
    observer m_observer;
};

/// Passes target events changes and holds its observer
template <typename E>
class event_tap_node final : public event_stream_node<E>
{
public:
    event_tap_node( const context& context, const events<E>& target, observer observer )
        : event_tap_node::event_stream_node( context )
        , m_target( target )
        , m_observer( std::move( observer ) )
    {
        this->attach_to( m_target );
    }

    ~event_tap_node() override
    {
        this->detach_from_all();
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        const auto& src_events = get_internals( m_target ).get_events();
        this->get_events() = src_events;

        return !this->get_events().empty() ? update_result::changed : update_result::unchanged;
    }

private:
    events<E> m_target;
    observer m_observer;
};

struct TapAdaptor : adaptor
{
    /*!
	 * @brief Create tapped copy for observed signal
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
    template <typename F, typename S>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const signal<S>& subject,
        F&& func,
        observe_policy policy = observe_policy::skip_current ) const
    {
        return create_wrapped_node<signal<S>, signal_tap_node<S>>( subject.get_context(),
            subject,
            observe_signal_impl( subject, std::forward<F>( func ), policy ) );
    }

    /*!
	 * @brief Create tapped copy for observed event stream
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
    template <typename F, typename E, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<E>& subject, const signal_pack<Deps...>& dep_pack, F&& func ) const
    {
        return create_wrapped_node<events<E>, event_tap_node<E>>( subject.get_context(),
            subject,
            observe_events_impl( subject, dep_pack, std::forward<F>( func ) ) );
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
	 * @brief Curried version of tap(T&& subject, F&& func, observe_policy policy)
	 */
    template <typename F>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( F&& func, observe_policy policy ) const
    {
        return make_partial<TapAdaptor>( std::forward<F>( func ), policy );
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
