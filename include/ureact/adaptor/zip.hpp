//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_ZIP_HPP
#define UREACT_ADAPTOR_ZIP_HPP

#include <deque>

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/base.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename E>
class event_stream_node;

template <typename... Values>
class event_zip_node final : public event_stream_node<std::tuple<Values...>>
{
public:
    explicit event_zip_node(
        context& context, const std::shared_ptr<event_stream_node<Values>>&... sources )
        : event_zip_node::event_stream_node( context )
        , m_slots( sources... )
    {
        ( this->attach_to( *sources ), ... );
    }

    ~event_zip_node() override
    {
        std::apply(
            [this]( slot<Values>&... slots ) { ( this->detach_from( *slots.source ), ... ); },
            m_slots );
    }

    UREACT_WARN_UNUSED_RESULT update_result update( turn_type& turn ) override
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

        return !this->m_events.empty() ? update_result::changed : update_result::unchanged;
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

struct ZipAdaptor : Adaptor
{
    /*!
	 * @brief Emit a tuple (e1,…,eN) for each complete set of values for sources 1...N
	 *
	 *  Each source slot has its own unbounded buffer queue that persistently stores incoming events.
	 *  For as long as all queues are not empty, one value is popped from each and emitted together as a tuple.
	 *
	 *  Semantically equivalent of ranges::zip
	 */
    template <typename Source, typename... Sources>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<Source>& source1, const events<Sources>&... sources ) const
    {
        static_assert( sizeof...( Sources ) >= 1, "zip: 2+ arguments are required" );

        context& context = source1.get_context();
        return events<std::tuple<Source, Sources...>>(
            std::make_shared<event_zip_node<Source, Sources...>>(
                context, source1.get_node(), sources.get_node()... ) );
    }
};

} // namespace detail

inline constexpr detail::ZipAdaptor zip;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_ZIP_HPP
