//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
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
#include <ureact/observer.hpp>
#include <ureact/signal_pack.hpp>
#include <ureact/type_traits.hpp>

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

template <typename E, typename F, typename... Args>
class add_observer_range_wrapper
{
public:
    template <typename InF, class = disable_if_same_t<InF, add_observer_range_wrapper>>
    explicit add_observer_range_wrapper( InF&& func )
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
    signal_observer_node(
        context& context, const std::shared_ptr<signal_node<S>>& subject, InF&& func )
        : signal_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<InF>( func ) )
    {
        this->attach_to( *subject );
    }

    UREACT_WARN_UNUSED_RESULT update_result update( turn_type& ) override
    {
        bool should_detach = false;

        if( auto p = m_subject.lock() )
        {
            if( std::invoke( m_func, p->value_ref() ) == observer_action::stop_and_detach )
            {
                should_detach = true;
            }
        }

        if( should_detach )
        {
            get_graph().queue_observer_for_detach( *this );
        }

        return update_result::unchanged;
    }

    void unregister_self() override
    {
        if( auto p = m_subject.lock() )
        {
            p->unregister_observer( this );
        }
    }

private:
    void detach_observer() override
    {
        if( auto p = m_subject.lock() )
        {
            detach_from( *p );
            m_subject.reset();
        }
    }

    std::weak_ptr<signal_node<S>> m_subject;
    F m_func;
};

template <typename E, typename F, typename... Deps>
class events_observer_node final : public observer_node
{
public:
    template <typename InF>
    events_observer_node( context& context,
        const std::shared_ptr<event_stream_node<E>>& subject,
        InF&& func,
        const std::shared_ptr<signal_node<Deps>>&... deps )
        : events_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<InF>( func ) )
        , m_deps( deps... )
    {
        this->attach_to( *subject );
        ( this->attach_to( *deps ), ... );
    }

    UREACT_WARN_UNUSED_RESULT update_result update( turn_type& turn ) override
    {
        bool should_detach = false;

        if( auto p = m_subject.lock() )
        {
            // Update of this node could be triggered from deps,
            // so make sure source doesn't contain events from last turn
            p->set_current_turn( turn );

            {
                should_detach
                    = std::apply(
                          [this, &p]( const std::shared_ptr<signal_node<Deps>>&... args ) {
                              return std::invoke(
                                  m_func, event_range<E>( p->events() ), args->value_ref()... );
                          },
                          m_deps )
                   == observer_action::stop_and_detach;
            }
        }

        if( should_detach )
        {
            get_graph().queue_observer_for_detach( *this );
        }

        return update_result::unchanged;
    }

    void unregister_self() override
    {
        if( auto p = m_subject.lock() )
        {
            p->unregister_observer( this );
        }
    }

private:
    using DepHolder = std::tuple<std::shared_ptr<signal_node<Deps>>...>;

    std::weak_ptr<event_stream_node<E>> m_subject;

    F m_func;
    DepHolder m_deps;

    void detach_observer() override
    {
        if( auto p = m_subject.lock() )
        {
            detach_from( *p );

            std::apply( detach_functor<events_observer_node>( *this ), m_deps );

            m_subject.reset();
        }
    }
};

template <typename InF, typename S>
auto observe_signal_impl( const signal<S>& subject, InF&& func ) -> observer
{
    static_assert( std::is_invocable_v<InF, S>,
        "Passed functor should be callable with S. See documentation for ureact::observe()" );

    using F = std::decay_t<InF>;
    using R = std::invoke_result_t<InF, S>;

    // If return value of passed function is void, add observer_action::next as
    // default return value.
    using Node = std::conditional_t<std::is_same_v<void, R>,
        signal_observer_node<S, add_observer_action_next_ret<F>>,
        signal_observer_node<S, F>>;

    const auto& subject_ptr = subject.get_node();

    std::unique_ptr<observer_node> node(
        new Node( subject.get_context(), subject_ptr, std::forward<InF>( func ) ) );
    observer_node* raw_node_ptr = node.get();

    subject_ptr->register_observer( std::move( node ) );

    return observer( raw_node_ptr, subject_ptr );
}

template <typename InF, typename E, typename... Deps>
auto observe_events_impl(
    const events<E>& subject, const signal_pack<Deps...>& dep_pack, InF&& func ) -> observer
{
    using F = std::decay_t<InF>;

    // clang-format off
    using wrapper_t =
        select_t<
            // observer_action func(event_range<E> range, const Deps& ...)
            condition<std::is_invocable_r_v<observer_action, F, event_range<E>, Deps...>,
                      F>,
            // observer_action func(const E&, const Deps& ...)
            condition<std::is_invocable_r_v<observer_action, F, E, Deps...>,
                      add_observer_range_wrapper<E, F, Deps...>>,
            // void func(event_range<E> range, const Deps& ...)
            condition<std::is_invocable_r_v<void, F, event_range<E>, Deps...>,
                      add_observer_action_next_ret<F>>,
            // void func(const E&, const Deps& ...)
            condition<std::is_invocable_r_v<void, F, E, Deps...>,
                      add_observer_range_wrapper<E, add_observer_action_next_ret<F>, Deps...>>,
            signature_mismatches>;
    // clang-format on

    static_assert( !std::is_same_v<wrapper_t, signature_mismatches>,
        "observe: Passed function does not match any of the supported signatures" );

    using Node = events_observer_node<E, wrapper_t, Deps...>;

    context& context = subject.get_context();

    auto node_builder = [&context, &subject, &func]( const signal<Deps>&... deps ) {
        return new Node(
            context, subject.get_node(), std::forward<InF>( func ), deps.get_node()... );
    };

    const auto& subject_node = subject.get_node();

    std::unique_ptr<observer_node> node( std::apply( node_builder, dep_pack.data ) );

    observer_node* raw_node = node.get();

    subject_node->register_observer( std::move( node ) );

    return observer( raw_node, subject_node );
}

struct ObserveAdaptor : Adaptor
{
    /*!
	 * @brief Create observer for signal
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
	 *
	 *  @note Resulting observer can be ignored. Lifetime of observer node will match subject signal's lifetime
	 */
    template <typename F, typename S>
    constexpr auto operator()( const signal<S>& subject, F&& func ) const
    {
        return observe_signal_impl( subject, std::forward<F>( func ) );
    }

    /*!
	 * @brief Create observer for temporary signal
	 *
	 *  Same as observe(const signal<S>& subject, F&& func),
	 *  but subject signal is about to die so caller must use result, otherwise observation isn't performed.
	 */
    template <typename F, typename S>
    UREACT_WARN_UNUSED_RESULT_MSG( "Observing the temporary so observer should be stored" )
    constexpr auto operator()( signal<S>&& subject, F&& func ) const
    {
        return observe_signal_impl( std::move( subject ), std::forward<F>( func ) );
    }

    /*!
	 * @brief Create observer for event stream
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
	 *  @note Resulting observer can be ignored. Lifetime of observer node will match subject signal's lifetime
	 *  @note The event_range<E> option allows to explicitly batch process single turn events
	 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
	 */
    template <typename F, typename E, typename... Deps>
    constexpr auto operator()(
        const events<E>& subject, const signal_pack<Deps...>& dep_pack, F&& func ) const
    {
        return observe_events_impl( subject, dep_pack, std::forward<F>( func ) );
    }

    /*!
	 * @brief Create observer for temporary event stream
	 *
	 *  Same as observe(const events<E>& subject, const signal_pack<Deps...>& dep_pack, F&& func),
	 *  but subject signal is about to die so caller must use result, otherwise observation isn't performed.
	 */
    template <typename F, typename E, typename... Deps>
    UREACT_WARN_UNUSED_RESULT_MSG( "Observing the temporary so observer should be stored" )
    constexpr auto operator()( events<E>&& subject,
        const signal_pack<Deps...>& dep_pack,
        F&& func ) const // TODO: check in tests
    {
        return observe_events_impl( std::move( subject ), dep_pack, std::forward<F>( func ) );
    }

    /*!
	 * @brief Create observer for event stream
	 *
	 *  Version without synchronization with additional signals
	 *
	 *  See observe(const events<E>& subject, const signal_pack<Deps...>& dep_pack, F&& func)
	 */
    template <typename F, typename E>
    constexpr auto operator()( const events<E>& subject, F&& func ) const
    {
        return operator()( subject, signal_pack<>{}, std::forward<F>( func ) );
    }

    /*!
	 * @brief Create observer for temporary event stream
	 *
	 *  Same as observe(const events<E>& subject, F&& func),
	 *  but subject signal is about to die so caller must use result, otherwise observation isn't performed.
	 */
    template <typename F, typename E>
    UREACT_WARN_UNUSED_RESULT_MSG( "Observing the temporary so observer should be stored" )
    constexpr auto operator()( events<E>&& subject, F&& func ) const // TODO: check in tests
    {
        return operator()( std::move( subject ), signal_pack<>{}, std::forward<F>( func ) );
    }

    /*!
	 * @brief Curried version of observe(T&& subject, F&& func)
	 */
    template <typename F>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( F&& func ) const // TODO: check in tests
    {
        // TODO: propagate [[nodiscard]] to closure operator() and operator|
        //       they should not be nodiscard for l-value arguments, but only for r-values like observe() does
        //       but maybe all observe() concept should be reconsidered before to not do feature that is possibly not needed
        return make_partial<ObserveAdaptor>( std::forward<F>( func ) );
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
