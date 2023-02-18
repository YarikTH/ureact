//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_FOLD_HPP
#define UREACT_ADAPTOR_FOLD_HPP

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/linker_functor.hpp>
#include <ureact/event_range.hpp>
#include <ureact/signal.hpp>
#include <ureact/signal_pack.hpp>
#include <ureact/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename E>
class event_stream_node;

template <typename E, typename S, typename F, typename... Args>
class add_fold_range_wrapper
{
public:
    template <typename InF, class = disable_if_same_t<InF, add_fold_range_wrapper>>
    explicit add_fold_range_wrapper( InF&& func )
        : m_func( std::forward<InF>( func ) )
    {}

    // TODO: possible optimization - move accum as much as possible. See std::accumulate
    // TODO: move 'typename... Args' here
    S operator()( event_range<E> range, S accum, const Args&... args )
    {
        for( const auto& e : range )
        {
            accum = std::invoke( m_func, e, accum, args... );
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
    template <typename InF, class = disable_if_same_t<InF, add_fold_by_ref_range_wrapper>>
    explicit add_fold_by_ref_range_wrapper( InF&& func )
        : m_func( std::forward<InF>( func ) )
    {}

    // TODO: move 'typename... Args' here
    void operator()( event_range<E> range, S& accum, const Args&... args )
    {
        for( const auto& e : range )
        {
            std::invoke( m_func, e, accum, args... );
        }
    }

private:
    F m_func;
};

template <typename S, typename E, typename F, typename... Deps>
class fold_node final : public signal_node<S>
{
public:
    template <typename InS, typename InF>
    fold_node( context& context,
        InS&& init,
        const std::shared_ptr<event_stream_node<E>>& events,
        InF&& func,
        const std::shared_ptr<signal_node<Deps>>&... deps )
        : fold_node::signal_node( context, std::forward<InS>( init ) )
        , m_events( events )
        , m_func( std::forward<InF>( func ) )
        , m_deps( deps... )
    {
        this->attach_to( *events );
        ( this->attach_to( *deps ), ... );
    }

    ~fold_node() override
    {
        this->detach_from( *m_events );

        std::apply( detach_functor<fold_node>( *this ), m_deps );
    }

    UREACT_WARN_UNUSED_RESULT update_result update( turn_type& turn ) override
    {
        m_events->set_current_turn( turn );

        if( m_events->events().empty() )
            return update_result::unchanged;

        if constexpr( std::is_invocable_r_v<S, F, event_range<E>, S, Deps...> )
        {
            return this->pulse_if_value_changed( std::apply(
                [this]( const std::shared_ptr<signal_node<Deps>>&... args ) {
                    UREACT_CALLBACK_GUARD( this->get_graph() );
                    return std::invoke( m_func,
                        event_range<E>( m_events->events() ),
                        this->m_value,
                        args->value_ref()... );
                },
                m_deps ) );
        }
        else if constexpr( std::is_invocable_r_v<void, F, event_range<E>, S&, Deps...> )
        {
            std::apply(
                [this]( const std::shared_ptr<signal_node<Deps>>&... args ) {
                    UREACT_CALLBACK_GUARD( this->get_graph() );
                    std::invoke( m_func,
                        event_range<E>( m_events->events() ),
                        this->m_value,
                        args->value_ref()... );
                },
                m_deps );

            // Always assume change
            return this->pulse_after_modify();
        }
        else
        {
            static_assert( always_false<S>, "Unsupported function signature" );
        }
    }

private:
    using DepHolder = std::tuple<std::shared_ptr<signal_node<Deps>>...>;

    std::shared_ptr<event_stream_node<E>> m_events;

    F m_func;
    DepHolder m_deps;
};

struct FoldAdaptor : Adaptor
{
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
    template <typename E, typename V, typename InF, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack, InF&& func ) const
    {
        using F = std::decay_t<InF>;
        using S = std::decay_t<V>;

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
                std::forward<InF>( func ),
                deps.get_node()... ) );
        };

        return std::apply( node_builder, dep_pack.data );
    }

    /*!
	 * @brief Curried version of fold(const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack, InF&& func)
	 */
    template <typename V, typename InF, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        V&& init, const signal_pack<Deps...>& dep_pack, InF&& func ) const
    {
        return make_partial<FoldAdaptor>(
            std::forward<V>( init ), dep_pack, std::forward<InF>( func ) );
    }

    /*!
	 * @brief Folds values from an event stream into a signal
	 *
	 *  Version without synchronization with additional signals
	 *
	 *  See fold(const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack, InF&& func)
	 */
    template <typename E, typename V, typename InF>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<E>& events, V&& init, InF&& func ) const
    {
        return operator()(
            events, std::forward<V>( init ), signal_pack<>{}, std::forward<InF>( func ) );
    }

    /*!
	 * @brief Curried version of fold(const events<E>& events, V&& init, InF&& func)
	 */
    template <typename V, typename InF>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( V&& init, InF&& func ) const
    {
        return make_partial<FoldAdaptor>( std::forward<V>( init ), std::forward<InF>( func ) );
    }
};

} // namespace detail

inline constexpr detail::FoldAdaptor fold;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_FOLD_HPP
