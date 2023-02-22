//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_PROCESS_HPP
#define UREACT_ADAPTOR_PROCESS_HPP

#include <functional>

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/linker_functor.hpp>
#include <ureact/event_emitter.hpp>
#include <ureact/event_range.hpp>
#include <ureact/events.hpp>
#include <ureact/signal_pack.hpp>

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
        this->attach_to( source->get_node_id() );
        ( this->attach_to( deps->get_node_id() ), ... );
    }

    ~event_processing_node() override
    {
        this->detach_from( m_source->get_node_id() );

        std::apply( detach_functor<event_processing_node>( *this ), m_deps );
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        if( !m_source->events().empty() )
        {
            std::apply(
                [this]( const std::shared_ptr<signal_node<Deps>>&... args ) {
                    UREACT_CALLBACK_GUARD( this->get_graph() );
                    std::invoke( m_func,
                        event_range<InE>( m_source->events() ),
                        event_emitter( this->events() ),
                        args->value_ref()... );
                },
                m_deps );
        }

        return !this->events().empty() ? update_result::changed : update_result::unchanged;
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
        return detail::create_wrapped_node<events<OutE>,
            event_processing_node<InE, OutE, F, Deps...>>(
            context, source.get_node(), std::forward<Op>( op ), deps.get_node()... );
    };

    return std::apply( node_builder, dep_pack.data );
}

template <typename OutE>
struct ProcessAdaptor : Adaptor
{
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
    template <typename InE, typename Op, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<InE>& source, const signal_pack<Deps...>& dep_pack, Op&& op ) const
    {
        return process_impl<OutE>( source, dep_pack, std::forward<Op>( op ) );
    }

    /*!
	 * @brief Curried version of process(const events<in_t>& source, Op&& op)
	 */
    template <typename Op, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const signal_pack<Deps...>& dep_pack, Op&& op ) const
    {
        return make_partial<ProcessAdaptor>( dep_pack, std::forward<Op>( op ) );
    }

    /*!
	 * @brief Create a new event stream by batch processing events from other stream
	 *
	 *  Version without synchronization with additional signals
	 *
	 *  See process(const events<InE>& source, const signal_pack<Deps...>& dep_pack, Op&& op)
	 */
    template <typename InE, typename Op>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<InE>& source, Op&& op ) const
    {
        return operator()( source, signal_pack<>{}, std::forward<Op>( op ) );
    }

    /*!
	 * @brief Curried version of process(const events<in_t>& source, Op&& op)
	 */
    template <typename Op>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Op&& op ) const
    {
        return make_partial<ProcessAdaptor>( std::forward<Op>( op ) );
    }
};

} // namespace detail

template <typename E>
inline constexpr detail::ProcessAdaptor<E> process;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_PROCESS_HPP
