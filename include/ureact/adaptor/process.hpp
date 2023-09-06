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

#include <ureact/core/adaptor.hpp>
#include <ureact/detail/linker_functor.hpp>
#include <ureact/events.hpp>
#include <ureact/utility/event_emitter.hpp>
#include <ureact/utility/event_range.hpp>
#include <ureact/utility/signal_pack.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename InE, typename OutE, typename Func, typename... Deps>
class event_processing_node final : public event_stream_node<OutE>
{
public:
    template <typename F>
    event_processing_node( const context& context,
        const events<InE>& source,
        F&& func,
        const signal_pack<Deps...>& deps )
        : event_processing_node::event_stream_node( context )
        , m_source( source )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps )
    {
        this->attach_to( source );
        this->attach_to( deps.data );
    }

    ~event_processing_node() override
    {
        this->detach_from_all();
    }

    UREACT_WARN_UNUSED_RESULT core::update_result update() override
    {
        const auto& src_events = get_internals( m_source ).get_events();
        if( !src_events.empty() )
        {
            std::apply(
                [this, &src_events]( const signal<Deps>&... args ) {
                    UREACT_CALLBACK_GUARD( this->get_graph() );
                    std::invoke( m_func,
                        event_range<InE>( src_events ),
                        get_internals( args ).value_ref()...,
                        event_emitter( this->get_events() ) );
                },
                m_deps.data );
        }

        return !this->get_events().empty() ? core::update_result::changed
                                           : core::update_result::unchanged;
    }

private:
    events<InE> m_source;
    Func m_func;
    signal_pack<Deps...> m_deps;
};

template <typename OutE>
struct ProcessAdaptor : core::adaptor
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
        using F = std::decay_t<Op>;

        const context& context = source.get_context();

        return core::create_wrapped_node<events<OutE>,
            event_processing_node<InE, OutE, F, Deps...>>(
            context, source, std::forward<Op>( op ), dep_pack );
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
