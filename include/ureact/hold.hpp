//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_HOLD_HPP
#define UREACT_HOLD_HPP

#include "closure.hpp"
#include "fold.hpp"

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Holds the most recent event in a signal
 *
 *  Creates a @ref signal with an initial value v = init.
 *  For received event values e1, e2, ... eN in events, it is updated to v = eN.
 */
template <typename V, typename E>
UREACT_WARN_UNUSED_RESULT auto hold( const events<E>& source, V&& init ) -> signal<E>
{
    return fold( source,
        std::forward<V>( init ),                  //
        []( event_range<E> range, const auto& ) { //
            return *range.rbegin();
        } );
}

/*!
 * @brief Curried version of hold()
 */
template <typename V>
UREACT_WARN_UNUSED_RESULT auto hold( V&& init )
{
    return closure{ [init = std::forward<V>( init )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return hold( std::forward<arg_t>( source ), std::move( init ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_HOLD_HPP
