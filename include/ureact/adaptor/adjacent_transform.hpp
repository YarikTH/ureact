//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_ADJACENT_TRANSFORM_HPP
#define UREACT_ADAPTOR_ADJACENT_TRANSFORM_HPP

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/adjacent_base.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <size_t N>
struct AdjacentTransformAdaptor : Adaptor
{
    static_assert( N >= 1 );

    template <typename EIn, typename F, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<EIn>& source, const signal_pack<Deps...>& dep_pack, F&& func ) const
    {
        using node_type = event_adjacent_node<adjacent_type::transform, EIn, N, F, Deps...>;
        using E = typename node_type::E;

        const context& context = source.get_context();
        return detail::create_wrapped_node<events<E>, node_type>(
            context, source, std::forward<F>( func ), dep_pack );
    }

    template <typename F, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const signal_pack<Deps...>& dep_pack, F&& func ) const
    {
        return make_partial<AdjacentTransformAdaptor<N>>( dep_pack, std::forward<F>( func ) );
    }

    template <typename EIn, typename F>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<EIn>& source, F&& func ) const
    {
        return AdjacentTransformAdaptor<N>::operator()(
            source, signal_pack<>{}, std::forward<F>( func ) );
    }

    template <typename F>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( F&& func ) const
    {
        return make_partial<AdjacentTransformAdaptor<N>>( std::forward<F>( func ) );
    }
};

} // namespace detail

/*!
 * @brief Takes an events<E> and func and emit a result of func(e1,…,eN) for last N event values
 * 
 * First N-1 events adjacent emits nothing, because there is no enough data yet to call a func.
 * 
 * Example of adjacent<3> with [](auto... ints) { return (... + ints); }:
 *  src = [1 2 3 4 5 6]
 *  e0  = skip
 *  e1  = skip
 *  e2  =      6
 *  e3  =        9
 *  e4  =          12
 *  e5  =            15
 * 
 *  Semantically equivalent of ranges::adjacent_transform
 */
template <size_t N>
inline constexpr detail::AdjacentTransformAdaptor<N> adjacent_transform;

UREACT_END_NAMESPACE

#endif //UREACT_ADAPTOR_ADJACENT_TRANSFORM_HPP
