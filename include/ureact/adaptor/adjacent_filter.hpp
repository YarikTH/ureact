//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_ADJACENT_FILTER_HPP
#define UREACT_ADAPTOR_ADJACENT_FILTER_HPP

#include <ureact/adaptor/detail/adjacent_base.hpp>
#include <ureact/detail/adaptor.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <size_t N>
struct AdjacentFilterAdaptor : adaptor
{
    static_assert( N >= 1 );

    template <typename EIn, typename F, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<EIn>& source, const signal_pack<Deps...>& dep_pack, F&& func ) const
    {
        using node_type = event_adjacent_node<adjacent_type::filter, EIn, N, F, Deps...>;
        using E = typename node_type::E;

        const context& context = source.get_context();
        return detail::create_wrapped_node<events<E>, node_type>(
            context, source, std::forward<F>( func ), dep_pack );
    }

    template <typename F, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const signal_pack<Deps...>& dep_pack, F&& func ) const
    {
        return make_partial<AdjacentFilterAdaptor<N>>( dep_pack, std::forward<F>( func ) );
    }

    template <typename EIn, typename F>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<EIn>& source, F&& func ) const
    {
        return AdjacentFilterAdaptor<N>::operator()(
            source, signal_pack<>{}, std::forward<F>( func ) );
    }

    template <typename F>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( F&& func ) const
    {
        return make_partial<AdjacentFilterAdaptor<N>>( std::forward<F>( func ) );
    }
};

} // namespace detail

/*!
 * @brief Takes an events<E> and func and after passing first N-1 events, emit next event only if result of func(e1,…,eN) for last N event values returns true
 * 
 * Example of adjacent<2> with [](const auto& last, const auto& current) { return current != last; }:
 *  src = [1 2 2 3 3 4]
 *  e0  =  1
 *  e1  =    2
 *  e2  =      skip
 *  e3  =        3
 *  e4  =          skip
 *  e5  =            4
 * 
 *  Semantically equivalent of ranges::adjacent_filter
 */
template <size_t N>
inline constexpr detail::AdjacentFilterAdaptor<N> adjacent_filter;

UREACT_END_NAMESPACE

#endif //UREACT_ADAPTOR_ADJACENT_FILTER_HPP
