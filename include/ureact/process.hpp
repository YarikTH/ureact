//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_PROCESS_HPP
#define UREACT_PROCESS_HPP

#include <functional>

#include <ureact/detail/closure.hpp>
#include <ureact/detail/linker_functor.hpp>
#include <ureact/event_emitter.hpp>
#include <ureact/event_range.hpp>
#include <ureact/events.hpp>
#include <ureact/signal_pack.hpp>
#include <ureact/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
class signal_node;

template <typename InE, typename OutE, typename Func, typename... Deps>
class event_processing_node final : public event_stream_node<OutE>
{
public:
    template <typename F>
    event_processing_node( context& context,
        const std::shared_ptr<event_stream_node<InE>>& source,
        F&& func,
        const std::shared_ptr<signal_node<Deps>>&... deps )
        : event_processing_node::event_stream_node( context )
        , m_source( source )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps... )
    {
        this->attach_to( *source );
        ( this->attach_to( *deps ), ... );
    }

    ~event_processing_node() override
    {
        this->detach_from( *m_source );

        std::apply( detach_functor<event_processing_node>( *this ), m_deps );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );
        // Update of this node could be triggered from deps,
        // so make sure source doesn't contain events from last turn
        m_source->set_current_turn( turn );

        if( !m_source->events().empty() )
        {
            std::apply(
                [this]( const std::shared_ptr<signal_node<Deps>>&... args ) {
                    UREACT_CALLBACK_GUARD( this->get_graph() );
                    std::invoke( m_func,
                        event_range<InE>( m_source->events() ),
                        event_emitter( this->m_events ),
                        args->value_ref()... );
                },
                m_deps );
        }

        this->pulse_if_has_events();
    }

private:
    using dep_holder_t = std::tuple<std::shared_ptr<signal_node<Deps>>...>;

    std::shared_ptr<event_stream_node<InE>> m_source;

    Func m_func;
    dep_holder_t m_deps;
};

template <typename OutE, typename InE, typename Op, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto process_impl(
    const events<InE>& source, const signal_pack<Deps...>& dep_pack, Op&& op ) -> events<OutE>
{
    using F = std::decay_t<Op>;

    context& context = source.get_context();

    auto node_builder = [&context, &source, &op]( const signal<Deps>&... deps ) {
        return events<OutE>( std::make_shared<event_processing_node<InE, OutE, F, Deps...>>(
            context, source.get_node(), std::forward<Op>( op ), deps.get_node()... ) );
    };

    return std::apply( node_builder, dep_pack.data );
}

} // namespace detail

/*!
 * @brief Create a new event stream by batch processing events from other stream
 *
 *  op is called with all events range from source in current turn.
 *  New events are emitted through "out".
 *  Synchronized values of signals in dep_pack are passed to op as additional arguments.
 *
 *  The signature of op should be equivalent to:
 *  * bool op(event_range<in_t> range, event_emitter<out_t> out, const Deps& ...)
 *
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 *  @note The type of outgoing events T has to be specified explicitly, i.e. process<T>(src, with(deps), op)
 */
template <typename OutE, typename InE, typename Op, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto process(
    const events<InE>& source, const signal_pack<Deps...>& dep_pack, Op&& op ) -> events<OutE>
{
    return detail::process_impl<OutE>( source, dep_pack, std::forward<Op>( op ) );
}

/*!
 * @brief Curried version of process(const events<in_t>& source, Op&& op)
 */
//template <typename OutE, typename Op, typename... Deps>
//UREACT_WARN_UNUSED_RESULT auto process( const signal_pack<Deps...>& dep_pack, Op&& op )
//{
//    return detail::closure{ [dep_pack = dep_pack, op = std::forward<Op>( op )]( auto&& source ) {
//        using arg_t = decltype( source );
//        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
//        return process<OutE>( std::forward<arg_t>( source ), dep_pack, op );
//    } };
//}

/*!
 * @brief Create a new event stream by batch processing events from other stream
 *
 *  Version without synchronization with additional signals
 *
 *  See process(const events<InE>& source, const signal_pack<Deps...>& dep_pack, Op&& op)
 */
template <typename OutE, typename InE, typename Op>
UREACT_WARN_UNUSED_RESULT auto process( const events<InE>& source, Op&& op ) -> events<OutE>
{
    return detail::process_impl<OutE>( source, signal_pack<>(), std::forward<Op>( op ) );
}

/*!
 * @brief Curried version of process(const events<in_t>& source, Op&& op)
 */
//template <typename OutE, typename Op>
//UREACT_WARN_UNUSED_RESULT auto process( Op&& op )
//{
//    return detail::closure{ [op = std::forward<Op>( op )]( auto&& source ) {
//        using arg_t = decltype( source );
//        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
//        return process<OutE>( std::forward<arg_t>( source ), op );
//    } };
//}

UREACT_END_NAMESPACE

#endif // UREACT_PROCESS_HPP
