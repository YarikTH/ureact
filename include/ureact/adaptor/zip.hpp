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
#include <ureact/events.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename... Values>
class event_zip_node final : public event_stream_node<std::tuple<Values...>>
{
public:
    explicit event_zip_node( const context& context, const events<Values>&... sources )
        : event_zip_node::event_stream_node( context )
        , m_slots( sources... )
    {
        ( this->attach_to( get_internals( sources ).get_node_id() ), ... );
    }

    ~event_zip_node() override
    {
        this->detach_from_all();
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        {
            // Move events into buffers
            std::apply( []( slot<Values>&... slots ) { ( fetch_buffer( slots ), ... ); }, m_slots );

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
                        this->get_events().emplace_back( slots.buffer.front()... );
                        ( slots.buffer.pop_front(), ... );
                    },
                    m_slots );
            }
        }

        return !this->get_events().empty() ? update_result::changed : update_result::unchanged;
    }

private:
    template <typename T>
    struct slot
    {
        explicit slot( const events<T>& source )
            : source( source )
        {}

        events<T> source;
        std::deque<T> buffer;
    };

    template <typename T>
    static void fetch_buffer( slot<T>& slot )
    {
        const auto& src_events = get_internals( slot.source ).get_events();

        slot.buffer.insert( slot.buffer.end(), src_events.begin(), src_events.end() );
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

        const context& context = source1.get_context();
        return detail::create_wrapped_node<events<std::tuple<Source, Sources...>>,
            event_zip_node<Source, Sources...>>( context, source1, sources... );
    }
};

} // namespace detail

inline constexpr detail::ZipAdaptor zip;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_ZIP_HPP
