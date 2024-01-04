//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_OBSERVE_HPP
#define UREACT_ADAPTOR_OBSERVE_HPP

#include <functional>

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/linker_functor.hpp>
#include <ureact/detail/observer_node.hpp>
#include <ureact/observer.hpp>
#include <ureact/utility/observe_policy.hpp>
#include <ureact/utility/observer_action.hpp>
#include <ureact/utility/signal_pack.hpp>
#include <ureact/utility/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

template <typename E>
class event_range;

namespace detail
{

template <typename E>
class event_stream_node;

/// Special wrapper to add specific return type to the void function
template <typename F, typename Ret, Ret return_value>
class add_default_return_value_wrapper
{
public:
    template <typename InF, class = disable_if_same_t<InF, add_default_return_value_wrapper>>
    explicit add_default_return_value_wrapper( InF&& func )
        : m_func( std::forward<InF>( func ) )
    {}

    template <typename... Args>
    UREACT_WARN_UNUSED_RESULT Ret operator()( Args&&... args )
    {
        std::invoke( m_func, std::forward<Args>( args )... );
        return return_value;
    }

private:
    F m_func;
};

template <class F>
using add_observer_action_next_ret
    = add_default_return_value_wrapper<F, observer_action, observer_action::next>;

template <typename Iter>
class add_observer_iterator_wrapper
{
public:
    template <typename InIter, class = disable_if_same_t<InIter, add_observer_iterator_wrapper>>
    explicit add_observer_iterator_wrapper( InIter&& iter )
        : m_iter( std::forward<InIter>( iter ) )
    {}

    template <typename Arg>
    UREACT_WARN_UNUSED_RESULT auto operator()( Arg&& arg )
    {
        *m_iter++ = std::forward<Arg>( arg );
        return observer_action::next;
    }

private:
    Iter m_iter;
};

template <typename Iter, typename Value, typename = void>
struct is_output_iterator : std::false_type
{};

template <typename Iter, class Value>
struct is_output_iterator<Iter,
    Value,
    std::void_t<decltype( *std::declval<Iter>()++ = std::declval<Value>() )>> : std::true_type
{};

template <typename Iter, class Value>
inline constexpr bool is_output_iterator_v = is_output_iterator<Iter, Value>::value;

template <typename E, typename F, typename... Args>
class add_observer_event_range_wrapper
{
public:
    template <typename InF, class = disable_if_same_t<InF, add_observer_event_range_wrapper>>
    explicit add_observer_event_range_wrapper( InF&& func )
        : m_func( std::forward<InF>( func ) )
    {}

    // NOTE: args can't be universal reference since its type is specified in class
    // NOTE: can't be const because m_func can be mutable
    observer_action operator()( event_range<E> range, const Args&... args )
    {
        for( const auto& e : range )
        {
            if( std::invoke( m_func, e, args... ) == observer_action::stop_and_detach )
            {
                return observer_action::stop_and_detach;
            }
        }

        return observer_action::next;
    }

private:
    F m_func;
};

template <typename S>
class signal_node;

template <typename S, typename F>
class signal_observer_node final : public observer_node
{
public:
    template <typename InF>
    signal_observer_node( const context& context, const signal<S>& subject, InF&& func )
        : signal_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<InF>( func ) )
    {
        this->attach_to( m_subject );
    }

    ~signal_observer_node() override
    {
        this->detach_from_all();
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        if( m_subject.is_valid() )
        {
            const observer_action action
                = std::invoke( m_func, get_internals( m_subject ).get_value() );

            if( action == observer_action::stop_and_detach )
                detach_observer();
        }

        return update_result::unchanged;
    }

private:
    void detach_observer() override
    {
        detach_from_all();

        m_subject = signal<S>{};
    }

    signal<S> m_subject;
    F m_func;
};

template <typename E, typename Func, typename... Deps>
class events_observer_node final : public observer_node
{
public:
    template <typename F>
    events_observer_node( const context& context,
        const events<E>& subject,
        F&& func,
        const signal_pack<Deps...>& deps )
        : events_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps )
    {
        this->attach_to( subject );
        this->attach_to( deps.data );
    }

    ~events_observer_node() override
    {
        this->detach_from_all();
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        if( m_subject.is_valid() )
        {
            const auto& src_events = get_internals( m_subject ).get_events();
            if( !src_events.empty() )
            {
                const observer_action action = std::apply(
                    [this, &src_events]( const signal<Deps>&... args ) {
                        return std::invoke( m_func,
                            event_range<E>( src_events ),
                            get_internals( args ).value_ref()... );
                    },
                    m_deps.data );

                if( action == observer_action::stop_and_detach )
                    detach_observer();
            }
        }

        return update_result::unchanged;
    }

private:
    void detach_observer() override
    {
        detach_from_all();

        m_subject = events<E>{};
    }

    events<E> m_subject;
    Func m_func;
    signal_pack<Deps...> m_deps;
};

template <typename InF, typename S>
auto observe_signal_impl(
    const signal<S>& subject, InF&& func, observe_policy policy = observe_policy::skip_current )
    -> observer
{
    using F = std::decay_t<InF>;

    // clang-format off
    using wrapper_t =
        select_t<
            // output_iterator
            condition<is_output_iterator_v<F, S>,
                      add_observer_iterator_wrapper<F>>,
            // observer_action func(const S&)
            condition<std::is_invocable_r_v<observer_action, F, S>,
                      F>,
            // void func(const S&)
            condition<std::is_invocable_r_v<void, F, S>,
                      add_observer_action_next_ret<F>>,
            signature_mismatches>;
    // clang-format on

    static_assert( !std::is_same_v<wrapper_t, signature_mismatches>,
        "observe: Passed function does not match any of the supported signatures" );

    observer obs = create_wrapped_node<observer, signal_observer_node<S, wrapper_t>>(
        subject.get_context(), subject, std::forward<InF>( func ) );

    // Call passed functor with current value, using direct call to signal_observer_node::update()
    // It allows not to duplicate observer_action handling logic
    if( policy == observe_policy::notify_current )
        std::ignore = get_internals( obs ).get_node_ptr()->update();

    return obs;
}

template <typename InF, typename E, typename... Deps>
auto observe_events_impl(
    const events<E>& subject, const signal_pack<Deps...>& dep_pack, InF&& func ) -> observer
{
    using F = std::decay_t<InF>;

    // clang-format off
    using wrapper_t =
        select_t<
            // output_iterator
            condition<is_output_iterator_v<F, E>,
                      add_observer_event_range_wrapper<E, add_observer_iterator_wrapper<F>, Deps...>>,
            // observer_action func(event_range<E> range, const Deps& ...)
            condition<std::is_invocable_r_v<observer_action, F, event_range<E>, Deps...>,
                      F>,
            // observer_action func(const E&, const Deps& ...)
            condition<std::is_invocable_r_v<observer_action, F, E, Deps...>,
                      add_observer_event_range_wrapper<E, F, Deps...>>,
            // void func(event_range<E> range, const Deps& ...)
            condition<std::is_invocable_r_v<void, F, event_range<E>, Deps...>,
                      add_observer_action_next_ret<F>>,
            // void func(const E&, const Deps& ...)
            condition<std::is_invocable_r_v<void, F, E, Deps...>,
                      add_observer_event_range_wrapper<E, add_observer_action_next_ret<F>, Deps...>>,
            signature_mismatches>;
    // clang-format on

    static_assert( !std::is_same_v<wrapper_t, signature_mismatches>,
        "observe: Passed function does not match any of the supported signatures" );

    const context& context = subject.get_context();

    return detail::create_wrapped_node<observer, events_observer_node<E, wrapper_t, Deps...>>(
        context, subject, std::forward<InF>( func ), dep_pack );
}

struct ObserveAdaptor : adaptor
{
    /*!
	 * @brief Create observer for signal
	 *
	 *  When the signal value S of subject changes, func is called
	 *
	 *  The signature of func should be equivalent to:
     *  * output_iterator
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
        return observe_signal_impl( subject, std::forward<F>( func ), policy );
    }

    /*!
	 * @brief Create observer for event stream
	 *
	 *  For every event e in subject, func is called.
	 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
	 *
	 *  The signature of func should be equivalent to:
     *  * output_iterator
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
        return observe_events_impl( subject, dep_pack, std::forward<F>( func ) );
    }

    /*!
	 * @brief Create observer for event stream
	 *
	 *  Version without synchronization with additional signals
	 *
	 *  See observe(const events<E>& subject, const signal_pack<Deps...>& dep_pack, F&& func)
	 */
    template <typename F, typename E>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<E>& subject, F&& func ) const
    {
        return operator()( subject, signal_pack<>{}, std::forward<F>( func ) );
    }

    /*!
	 * @brief Curried version of observe(T&& subject, F&& func)
	 */
    template <typename F>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( F&& func ) const // TODO: check in tests
    {
        return make_partial<ObserveAdaptor>( std::forward<F>( func ) );
    }

    /*!
	 * @brief Curried version of observe(T&& subject, F&& func, observe_policy policy)
	 */
    template <typename F>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( F&& func,
        observe_policy policy ) const // TODO: check in tests
    {
        return make_partial<ObserveAdaptor>( std::forward<F>( func ), policy );
    }

    /*!
	 * @brief Curried version of observe(T&& subject, const signal_pack<Deps...>& dep_pack, F&& func)
	 */
    template <typename F, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const signal_pack<Deps...>& dep_pack, F&& func ) const // TODO: check in tests
    {
        return make_partial<ObserveAdaptor>( dep_pack, std::forward<F>( func ) );
    }
};

} // namespace detail

inline constexpr detail::ObserveAdaptor observe;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_OBSERVE_HPP
