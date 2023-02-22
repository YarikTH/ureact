//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_MERGE_HPP
#define UREACT_ADAPTOR_MERGE_HPP

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/base.hpp>
#include <ureact/events.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename E>
using event_stream_node_ptr_t = std::shared_ptr<event_stream_node<E>>;

template <typename E, typename... Sources>
class event_merge_node final : public event_stream_node<E>
{
public:
    explicit event_merge_node(
        context& context, const std::shared_ptr<event_stream_node<Sources>>&... sources )
        : event_merge_node::event_stream_node( context )
        , m_sources( sources... )
    {
        ( this->attach_to( sources->get_node_id() ), ... );
    }

    ~event_merge_node() override
    {
        std::apply(
            [this]( const event_stream_node_ptr_t<Sources>&... sources ) {
                ( this->detach_from( sources->get_node_id() ), ... );
            },
            m_sources );
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        std::apply(
            [this]( const event_stream_node_ptr_t<Sources>&... sources ) {
                ( this->copy_events_from( *sources ), ... );
            },
            m_sources );

        return !this->events().empty() ? update_result::changed : update_result::unchanged;
    }

private:
    template <typename V>
    void copy_events_from( const event_stream_node<V>& src )
    {
        this->events().insert( this->events().end(), src.events().begin(), src.events().end() );
    }

    std::tuple<event_stream_node_ptr_t<Sources>...> m_sources;
};

struct MergeAdaptor : Adaptor
{
    /*!
	 * @brief Emit all events in source1, ... sources
	 *
	 *  @warning Not to be confused with std::merge() or ranges::merge()
	 */
    template <typename Source, typename... Sources, typename E = Source>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<Source>& source1, const events<Sources>&... sources ) const
    {
        static_assert( sizeof...( Sources ) >= 1, "merge: 2+ arguments are required" );

        context& context = source1.get_context();
        return detail::create_wrapped_node<events<E>, event_merge_node<E, Source, Sources...>>(
            context, source1.get_node(), sources.get_node()... );
    }
};

} // namespace detail

inline constexpr detail::MergeAdaptor merge;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_MERGE_HPP
