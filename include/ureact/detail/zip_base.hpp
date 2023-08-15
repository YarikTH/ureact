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
#include <functional>
#include <tuple>

#include <ureact/detail/defines.hpp>
#include <ureact/events.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename F, typename... Values>
struct zip_result
{
    using func_type = std::conditional_t< //
        std::is_same_v<F, unit>,          //
        decltype( std::make_tuple<Values...> ),
        F>;
    using type = std::invoke_result_t<func_type, Values...>;
};

template <typename F, typename... Values>
using zip_result_t = typename zip_result<F, Values...>::type;

template <typename F, typename... Values>
class event_zip_node final : public event_stream_node<zip_result_t<F, Values...>>
{
public:
    using E = zip_result_t<F, Values...>;

    template <typename InF>
    explicit event_zip_node( const context& context, InF&& func, const events<Values>&... sources )
        : event_zip_node::event_stream_node( context )
        , m_func( std::forward<InF>( func ) )
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
        fetch_buffers();

        auto& events = this->get_events();

        while( are_all_slots_ready() )
            pop_slots_to_emit_event( [this, &events]( Values&&... values ) { //
                std::ignore = this; // [[maybe_unused]] analog for lambda capture

                if constexpr( std::is_same_v<F, unit> )
                    events.emplace_back( std::forward<Values>( values )... );
                else
                    events.push_back( std::invoke( m_func, std::forward<Values>( values )... ) );
            } );

        return !this->get_events().empty() ? update_result::changed : update_result::unchanged;
    }

private:
    template <typename T>
    class slot
    {
    public:
        explicit slot( events<T> source )
            : m_source( std::move( source ) )
        {}

        void fetch_buffer()
        {
            const auto& src_events = get_internals( m_source ).get_events();
            m_buffer.insert( m_buffer.end(), src_events.begin(), src_events.end() );
        }

        [[nodiscard]] bool is_ready() const
        {
            return !m_buffer.empty();
        }

        T&& front()
        {
            return std::move( m_buffer.front() );
        }

        void pop_front()
        {
            m_buffer.pop_front();
        }

    private:
        events<T> m_source;
        std::deque<T> m_buffer;
    };

    // Move source events into buffers
    void fetch_buffers()
    {
        std::apply(
            []( slot<Values>&... slots ) {
                ( slots.fetch_buffer(), ... ); //
            },
            m_slots );
    }

    // Check if all slots are ready (has value)
    [[nodiscard]] bool are_all_slots_ready() const
    {
        return std::apply(
            []( const slot<Values>&... slots ) {
                return ( slots.is_ready() && ... ); //
            },
            m_slots );
    }

    // Pop values from buffers and emit value
    template <typename EmitEventT>
    void pop_slots_to_emit_event( const EmitEventT& emit_event )
    {
        std::apply(
            [&emit_event]( slot<Values>&... slots ) {
                emit_event( std::move( slots.front() )... );
                ( slots.pop_front(), ... );
            },
            m_slots );
    }

    F m_func;
    std::tuple<slot<Values>...> m_slots;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_ZIP_BASE_HPP
