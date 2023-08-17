//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_ADJACENT_HPP
#define UREACT_ADAPTOR_ADJACENT_HPP

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/adjacent_base.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <size_t N>
struct AdjacentClosure : AdaptorClosure
{
    static_assert( N >= 1 );

    template <typename EIn>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<EIn>& source ) const
    {
        using node_type = event_adjacent_node<adjacent_type::normal, EIn, N>;
        using E = typename node_type::E;

        const context& context = source.get_context();
        return detail::create_wrapped_node<events<E>, node_type>(
            context, source, unit{}, signal_pack<>{} );
    }
};

} // namespace detail

/*!
 * @brief Takes an events<E> and emit a tuple (e1,…,eN) of last N event values
 * 
 * First N-1 events adjacent emits nothing, because there is no enough data yet to emit a tuple.
 * 
 * Example of adjacent<3>:
 *  src = [1 2 3 4 5 6]
 *  e0  = skip
 *  e1  = skip
 *  e2  = [1 2 3]
 *  e3  =   [2 3 4]
 *  e4  =     [3 4 5]
 *  e5  =       [4 5 6]
 * 
 *  Semantically equivalent of ranges::adjacent
 */
template <size_t N>
inline constexpr detail::AdjacentClosure<N> adjacent;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_ADJACENT_HPP
