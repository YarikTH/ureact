//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_DETAIL_ADJACENT_BASE_HPP
#define UREACT_ADAPTOR_DETAIL_ADJACENT_BASE_HPP

#include <array>
#include <functional>
#include <tuple>

#include <ureact/detail/defines.hpp>
#include <ureact/events.hpp>
#include <ureact/utility/signal_pack.hpp>
#include <ureact/utility/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

// uniform_tuple code is based on https://stackoverflow.com/a/40001277/9323999
template <typename T, typename... Null>
struct type_convert
{
    using type = T;
};

template <typename T, typename... Null>
using type_convert_t = typename type_convert<T, Null...>::type;

template <typename T, size_t N, typename = std::make_index_sequence<N>>
struct uniform_tuple;

template <typename T, size_t N, size_t... S>
struct uniform_tuple<T, N, std::index_sequence<S...>>
{
    using type = std::tuple<type_convert_t<T, decltype( S )>...>;
};

template <typename T, size_t N>
using uniform_tuple_t = typename uniform_tuple<T, N>::type;

enum class adjacent_type
{
    normal,
    transform,
    filter,
};

template <adjacent_type Type, typename E, size_t N, typename F, typename... Deps>
struct adjacent_result;

template <typename E, size_t N, typename F, typename... Deps>
struct adjacent_result<adjacent_type::normal, E, N, F, Deps...>
{
    using type = uniform_tuple_t<E, N>;
};

template <typename E, size_t N, typename F, typename... Deps>
struct adjacent_result<adjacent_type::transform, E, N, F, Deps...>
{
private:
    template <typename Fun>
    static auto test( Fun&& func )
    {
        return test( std::forward<Fun>( func ), std::make_index_sequence<N>{} );
    }

    template <typename Fun, std::size_t... I>
    static auto test( Fun&& func, std::integer_sequence<size_t, I...> )
    {
        return std::apply(
            [&func]( const Deps&... args ) {
                std::array<E, N> buffer;
                return std::invoke( std::forward<Fun>( func ), buffer[I]..., args... );
            },
            std::tuple<Deps...>{} );
    }

public:
    using type = std::decay_t<decltype( adjacent_result::test( std::declval<F>() ) )>;
};

template <typename E, size_t N, typename F, typename... Deps>
struct adjacent_result<adjacent_type::filter, E, N, F, Deps...>
{
    using type = E;
};

template <adjacent_type Type, typename E, size_t N, typename F = unit, typename... Deps>
using adjacent_result_t = typename adjacent_result<Type, E, N, F, Deps...>::type;

template <adjacent_type Type, typename EIn, size_t N, typename F = unit, typename... Deps>
class event_adjacent_node final
    : public event_stream_node<adjacent_result_t<Type, EIn, N, F, Deps...>>
{
public:
    using E = adjacent_result_t<Type, EIn, N, F, Deps...>;

    template <typename InF>
    explicit event_adjacent_node( const context& context,
        const events<EIn>& source,
        InF&& func,
        const signal_pack<Deps...>& deps )
        : event_adjacent_node::event_stream_node( context )
        , m_source( source )
        , m_func( std::forward<InF>( func ) )
        , m_deps( deps )
    {
        this->attach_to( m_source );
        this->attach_to( deps.data );
    }

    ~event_adjacent_node() override
    {
        this->detach_from_all();
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        const auto& src_events = get_internals( m_source ).get_events();
        for( const EIn& e : src_events )
        {
            process_input_event( e );
            emit_event();
        }

        return !this->get_events().empty() ? update_result::changed : update_result::unchanged;
    }

private:
    void process_input_event( const EIn& e )
    {
        if( m_bufferSize < N )
        {
            m_buffer[m_bufferSize++] = e;
        }
        else
        {
            // moving shift array elements left
            for( size_t i = 0; i < N - 1; ++i )
                m_buffer[i] = std::move( m_buffer[i + 1] );
            m_buffer.back() = e;
        }
    }

    void emit_event()
    {
        emit_event( std::make_index_sequence<N>{} );
    }

    template <std::size_t... I>
    void emit_event( std::integer_sequence<size_t, I...> )
    {
        auto& events = this->get_events();

        if constexpr( Type == adjacent_type::normal )
        {
            if( m_bufferSize == N )
                events.emplace_back( m_buffer[I]... );
        }
        else
        {
            const auto invoke_helper = [this]() {
                return std::apply(
                    [this]( const signal<Deps>&... args ) {
                        UREACT_CALLBACK_GUARD( this->get_graph() );
                        return std::invoke(
                            m_func, m_buffer[I]..., get_internals( args ).value_ref()... );
                    },
                    m_deps.data );
            };

            if constexpr( Type == adjacent_type::transform )
            {
                if( m_bufferSize == N )
                    events.push_back( invoke_helper() );
            }
            else if constexpr( Type == adjacent_type::filter )
            {
                if( m_bufferSize < N || invoke_helper() )
                    events.push_back( m_buffer[m_bufferSize - 1] );
            }
            else
            {
                static_assert( always_false<EIn>, "Invalid adjacent_type" );
            }
        }
    }

    std::array<EIn, N> m_buffer;
    size_t m_bufferSize = 0;
    events<EIn> m_source;
    F m_func;
    signal_pack<Deps...> m_deps;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_DETAIL_ADJACENT_BASE_HPP
