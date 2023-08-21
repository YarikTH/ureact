//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_TRANSFORM_5_HPP
#define UREACT_ADAPTOR_TRANSFORM_5_HPP

#include <functional>

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/deduce_result_type.hpp>
#include <ureact/detail/node_base.hpp>
#include <ureact/detail/synced_adaptor_base.hpp>
#include <ureact/events.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename InE, typename OutE, typename... Deps>
class event_transform5_node final : public event_stream_node<OutE>
{
public:
    using F = std::function<OutE( const InE&, const Deps&... )>;

    event_transform5_node( const context& context,
        const events<InE>& source,
        F func,
        const signal_pack<Deps...>& deps )
        : event_transform5_node::event_stream_node( context )
        , m_source( source )
        , m_func( std::move( func ) )
        , m_deps( deps )
    {
        this->attach_to( source );
        this->attach_to( deps.data );
    }

    ~event_transform5_node() override
    {
        this->detach_from_all();
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        const auto& src_events = get_internals( m_source ).get_events();
        if( !src_events.empty() )
        {
            std::apply(
                [this, &src_events]( const signal<Deps>&... args ) {
                    for( const InE& e : src_events )
                    {
                        UREACT_CALLBACK_GUARD( this->get_graph() );
                        this->get_events().push_back( //
                            m_func( e, get_internals( args ).value_ref()... ) );
                    }
                },
                m_deps.data );
        }

        return !this->get_events().empty() ? update_result::changed : update_result::unchanged;
    }

private:
    events<InE> m_source;
    F m_func;
    signal_pack<Deps...> m_deps;
};

template <typename EIn = void>
struct Transform5Adaptor : SyncedAdaptorBase<Transform5Adaptor<EIn>>
{
    /*!
	 * @brief Create a new event stream that transforms events from other stream
	 *
	 *  For every event e in source, emit t = func(e, deps...).
	 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
	 *
	 *  The signature of func should be equivalent to:
	 *  * T func(const E&, const Deps& ...)
	 *
	 *  Semantically equivalent of ranges::transform
	 *
	 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
	 */
    template <typename InE, typename InF, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<InE>& source, const signal_pack<Deps...>& dep_pack, InF&& func ) const
    {
        using OutE = deduce_result_type<EIn, InF, InE, Deps...>;

        const context& context = source.get_context();

        return detail::create_wrapped_node<events<OutE>, event_transform5_node<InE, OutE, Deps...>>(
            context, source, std::forward<InF>( func ), dep_pack );
    }

    using SyncedAdaptorBase<Transform5Adaptor<EIn>>::operator();
};

} // namespace detail

/*!
 * @brief Create a new event stream that transforms events from other stream
 *
 *  Type of resulting events should be explicitly specified.
 */
template <typename EIn = void>
inline constexpr detail::Transform5Adaptor<EIn> transform5_as;

/*!
 * @brief Create a new event stream that transforms events from other stream
 */
inline constexpr detail::Transform5Adaptor<> transform5;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_TRANSFORM_5_HPP
