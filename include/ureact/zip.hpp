//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ZIP_HPP
#define UREACT_ZIP_HPP

#include <deque>

#include "ureact.hpp"

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename... Values>
class event_zip_node final : public event_stream_node<std::tuple<Values...>>
{
public:
    explicit event_zip_node(
        context& context, const std::shared_ptr<event_stream_node<Values>>&... sources )
        : event_zip_node::event_stream_node( context )
        , m_slots( sources... )
    {
        ( this->get_graph().on_node_attach( *this, *sources ), ... );
    }

    ~event_zip_node() override
    {
        std::apply(
            [this]( slot<Values>&... slots ) {
                ( this->get_graph().on_node_detach( *this, *slots.source ), ... );
            },
            m_slots );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );

        {
            // Move events into buffers
            std::apply( [&turn]( slot<Values>&... slots ) { ( fetch_buffer( turn, slots ), ... ); },
                m_slots );

            while( true )
            {
                bool is_ready = true;

                // All slots ready?
                std::apply(
                    [&is_ready]( slot<Values>&... slots ) {
                        // Todo: combine return values instead
                        ( check_slot( slots, is_ready ), ... );
                    },
                    m_slots );

                if( !is_ready )
                {
                    break;
                }

                // Pop values from buffers and emit tuple
                std::apply(
                    [this]( slot<Values>&... slots ) {
                        this->m_events.emplace_back( slots.buffer.front()... );
                        ( slots.buffer.pop_front(), ... );
                    },
                    m_slots );
            }
        }

        if( !this->m_events.empty() )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    template <typename T>
    struct slot
    {
        explicit slot( const std::shared_ptr<event_stream_node<T>>& source )
            : source( source )
        {}

        std::shared_ptr<event_stream_node<T>> source;
        std::deque<T> buffer;
    };

    template <typename T>
    static void fetch_buffer( turn_type& turn, slot<T>& slot )
    {
        slot.source->set_current_turn( turn );

        slot.buffer.insert(
            slot.buffer.end(), slot.source->events().begin(), slot.source->events().end() );
    }

    template <typename T>
    static void check_slot( slot<T>& slot, bool& is_ready )
    {
        auto t = is_ready && !slot.buffer.empty();
        is_ready = t;
    }

    std::tuple<slot<Values>...> m_slots;
};

} // namespace detail

/*!
 * @brief Emit a tuple (e1,…,eN) for each complete set of values for sources 1...N
 *
 *  Each source slot has its own unbounded buffer queue that persistently stores incoming events.
 *  For as long as all queues are not empty, one value is popped from each and emitted together as a tuple.
 *
 *  Semantically equivalent of ranges::zip
 */
template <typename Source, typename... Sources>
UREACT_WARN_UNUSED_RESULT auto zip( const events<Source>& source1,
    const events<Sources>&... sources ) -> events<std::tuple<Source, Sources...>>
{
    static_assert( sizeof...( Sources ) >= 1, "zip: 2+ arguments are required" );

    context& context = source1.get_context();
    return events<std::tuple<Source, Sources...>>(
        std::make_shared<detail::event_zip_node<Source, Sources...>>(
            context, source1.get_node(), sources.get_node()... ) );
}

UREACT_END_NAMESPACE

#endif // UREACT_ZIP_HPP