//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_FOLD_HPP
#define UREACT_FOLD_HPP

#include "ureact.hpp"

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename E, typename S, typename F, typename... Args>
class add_fold_range_wrapper
{
public:
    template <typename FIn, class = disable_if_same_t<FIn, add_fold_range_wrapper>>
    explicit add_fold_range_wrapper( FIn&& func )
        : m_func( std::forward<FIn>( func ) )
    {}

    // TODO: possible optimization - move accum as much as possible. See std::accumulate
    // TODO: move 'typename... Args' here
    S operator()( event_range<E> range, S accum, const Args&... args )
    {
        for( const auto& e : range )
        {
            accum = m_func( e, accum, args... );
        }

        return accum;
    }

private:
    F m_func;
};

template <typename E, typename S, typename F, typename... Args>
class add_fold_by_ref_range_wrapper
{
public:
    template <typename FIn, class = disable_if_same_t<FIn, add_fold_by_ref_range_wrapper>>
    explicit add_fold_by_ref_range_wrapper( FIn&& func )
        : m_func( std::forward<FIn>( func ) )
    {}

    // TODO: move 'typename... Args' here
    void operator()( event_range<E> range, S& accum, const Args&... args )
    {
        for( const auto& e : range )
        {
            m_func( e, accum, args... );
        }
    }

private:
    F m_func;
};

template <typename S, typename E, typename F, typename... DepValues>
class fold_node final : public signal_node<S>
{
public:
    template <typename InS, typename InF>
    fold_node( context& context,
        InS&& init,
        const std::shared_ptr<event_stream_node<E>>& events,
        InF&& func,
        const std::shared_ptr<signal_node<DepValues>>&... deps )
        : fold_node::signal_node( context, std::forward<InS>( init ) )
        , m_events( events )
        , m_func( std::forward<InF>( func ) )
        , m_deps( deps... )
    {
        this->get_graph().on_node_attach( *this, *events );
        ( this->get_graph().on_node_attach( *this, *deps ), ... );
    }

    ~fold_node() override
    {
        this->get_graph().on_node_detach( *this, *m_events );

        std::apply( detach_functor<fold_node, std::shared_ptr<signal_node<DepValues>>...>( *this ),
            m_deps );
    }

    void tick( turn_type& turn ) override
    {
        m_events->set_current_turn( turn );

        if( m_events->events().empty() )
            return;

        if constexpr( std::is_invocable_r_v<S, F, event_range<E>, S, DepValues...> )
        {
            this->pulse_if_value_changed( std::apply(
                [this]( const std::shared_ptr<signal_node<DepValues>>&... args ) {
                    return m_func(
                        event_range<E>( m_events->events() ), this->m_value, args->value_ref()... );
                },
                m_deps ) );
        }
        else if constexpr( std::is_invocable_r_v<void, F, event_range<E>, S&, DepValues...> )
        {
            std::apply(
                [this]( const std::shared_ptr<signal_node<DepValues>>&... args ) {
                    m_func(
                        event_range<E>( m_events->events() ), this->m_value, args->value_ref()... );
                },
                m_deps );

            // Always assume change
            this->pulse_after_modify();
        }
        else
        {
            static_assert( always_false<S>, "Unsupported function signature" );
        }
    }

private:
    using DepHolder = std::tuple<std::shared_ptr<signal_node<DepValues>>...>;

    std::shared_ptr<event_stream_node<E>> m_events;

    F m_func;
    DepHolder m_deps;
};

template <typename E, typename V, typename FIn, typename... Deps, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto fold_impl(
    const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack, FIn&& func )
    -> signal<S>
{
    using F = std::decay_t<FIn>;

    // clang-format off
    using Node =
        select_t<
            // S func(const S&, event_range<E> range, const Deps& ...)
            condition<std::is_invocable_r_v<S, F, event_range<E>, S, Deps...>,
                                  fold_node<S, E, F, Deps...>>,
            // S func(const S&, const E&, const Deps& ...)
            condition<std::is_invocable_r_v<S, F, E, S, Deps...>,
                                  fold_node<S, E, add_fold_range_wrapper<E, S, F, Deps...>, Deps...>>,
            // void func(S&, event_range<E> range, const Deps& ...)
            condition<std::is_invocable_r_v<void, F, event_range<E>, S&, Deps...>,
                                  fold_node<S, E, F, Deps...>>,
            // void func(S&, const E&, const Deps& ...)
            condition<std::is_invocable_r_v<void, F, E, S&, Deps...>,
                                  fold_node<S, E, add_fold_by_ref_range_wrapper<E, S, F, Deps...>, Deps...>>,
            signature_mismatches>;
    // clang-format on

    static_assert( !std::is_same_v<Node, signature_mismatches>,
        "fold: Passed function does not match any of the supported signatures" );

    context& context = events.get_context();

    auto node_builder = [&context, &events, &init, &func]( const signal<Deps>&... deps ) {
        return signal<S>( std::make_shared<Node>( context,
            std::forward<V>( init ),
            events.get_node(),
            std::forward<FIn>( func ),
            deps.get_node()... ) );
    };

    return std::apply( node_builder, dep_pack.data );
}

} // namespace detail

/*!
 * @brief Folds values from an event stream into a signal
 *
 *  Iteratively combines signal value with values from event stream.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of func should be equivalent to:
 *  * S func(const E& event, const S& accum, const Deps& ...)
 *  * S func(event_range<E> range, const S& accum, const Deps& ...)
 *  * void func(const E& event, S& accum, const Deps& ...)
 *  * void func(event_range<E> range, S& accum, const Deps& ...)
 *
 *  The fold parameters:
 *    [const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack]
 *  match the corresponding arguments of the given function
 *    [const E& event_value, const S& accumulator, const Deps& ...deps]
 *
 *  Creates a signal with an initial value v = init.
 *  * If the return type of func is S: For every received event e in events, v is updated to v = func(e,v, deps).
 *  * If the return type of func is void: For every received event e in events,
 *    v is passed by non-cost reference to func(v, e, deps), making it mutable.
 *    This variant can be used if copying and comparing S is prohibitively expensive.
 *    Because the old and new values cannot be compared, updates will always trigger a change.
 *
 *  @note order of arguments is inverse compared with std::accumulate() to correspond fold parameters
 *  @note The event_range<E> option allows to explicitly batch process single turn events
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename E, typename V, typename FIn, typename... Deps, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto fold(
    const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack, FIn&& func )
    -> signal<S>
{
    return fold_impl( events, std::forward<V>( init ), dep_pack, std::forward<FIn>( func ) );
}

/*!
 * @brief Curried version of fold(const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack, FIn&& func)
 */
template <typename V, typename FIn, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto fold( V&& init, const signal_pack<Deps...>& dep_pack, FIn&& func )
{
    return closure{ [init = std::forward<V>( init ),
                        deps = dep_pack.store(),
                        func = std::forward<FIn>( func )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return fold(
            std::forward<arg_t>( source ), std::move( init ), signal_pack<Deps...>( deps ), func );
    } };
}

/*!
 * @brief Folds values from an event stream into a signal
 *
 *  Version without synchronization with additional signals
 *
 *  See fold(const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack, FIn&& func)
 */
template <typename E, typename V, typename FIn, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto fold( const events<E>& events, V&& init, FIn&& func ) -> signal<S>
{
    return fold_impl( events, std::forward<V>( init ), signal_pack<>(), std::forward<FIn>( func ) );
}

/*!
 * @brief Curried version of fold(const events<E>& events, V&& init, FIn&& func)
 */
template <typename V, typename FIn>
UREACT_WARN_UNUSED_RESULT auto fold( V&& init, FIn&& func )
{
    return closure{
        [init = std::forward<V>( init ), func = std::forward<FIn>( func )]( auto&& source ) {
            using arg_t = decltype( source );
            static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
            return fold( std::forward<arg_t>( source ), std::move( init ), func );
        } };
}

UREACT_END_NAMESPACE

#endif // UREACT_FOLD_HPP
