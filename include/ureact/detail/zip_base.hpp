//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_ZIP_BASE_HPP
#define UREACT_DETAIL_ZIP_BASE_HPP

#include <deque>
#include <tuple>

#include <ureact/detail/defines.hpp>
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
        this->attach_to( sources... );
    }

    ~event_zip_node() override
    {
        this->detach_from_all();
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        // Move events into buffers
        // TODO: move to method
        std::apply(
            []( slot<Values>&... slots ) {
                ( fetch_buffer( slots ), ... ); //
            },
            m_slots );

        // TODO: move to method
        const auto is_ready = [this]() {
            return std::apply(
                []( const slot<Values>&... slots ) {
                    return ( !slots.buffer.empty() && ... ); //
                },
                m_slots );
        };

        while( is_ready() )
        {
            // Pop values from buffers and emit tuple
            // TODO: move to method
            std::apply(
                [this]( slot<Values>&... slots ) {
                    this->get_events().emplace_back( slots.buffer.front()... );
                    ( slots.buffer.pop_front(), ... );
                },
                m_slots );
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

    std::tuple<slot<Values>...> m_slots;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_ZIP_BASE_HPP
