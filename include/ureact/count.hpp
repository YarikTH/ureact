//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_COUNT_HPP
#define UREACT_COUNT_HPP

#include "fold.hpp"

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Counts amount of received events into signal<S>
 *
 *  Type of resulting signal can be explicitly specified.
 *  Value type should be default constructing and prefix incremented
 *
 *  @warning Not to be confused with std::count(from, to, value)
 */
template <typename S = size_t, class E>
UREACT_WARN_UNUSED_RESULT auto count( const events<E>& source ) -> signal<S>
{
    return fold( source,
        S{},                       //
        []( const E&, S& accum ) { //
            ++accum;
        } );
}

/*!
 * @brief Curried version of count(const events<E>& source)
 */
template <typename S = size_t>
UREACT_WARN_UNUSED_RESULT auto count()
{
    return closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return count<S>( std::forward<arg_t>( source ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_COUNT_HPP
