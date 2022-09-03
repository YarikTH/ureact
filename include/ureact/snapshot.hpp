//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_SNAPSHOT_HPP
#define UREACT_SNAPSHOT_HPP

#include "ureact.hpp"

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Sets the signal value to the value of a target signal when an event is received
 *
 *  Creates a signal with value v = target.get().
 *  The value is set on construction and updated only when receiving an event from trigger
 */
template <typename S, typename E>
UREACT_WARN_UNUSED_RESULT auto snapshot( const events<E>& trigger, const signal<S>& target )
    -> signal<S>
{
    return fold( trigger,
        target.get(),
        with( target ),
        []( event_range<E> range, const S&, const S& value ) { //
            return value;
        } );
}

/*!
 * @brief Curried version of snapshot() algorithm used for "pipe" syntax
 */
template <typename S>
UREACT_WARN_UNUSED_RESULT auto snapshot( const signal<S>& target )
{
    return closure{ [target = target]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return snapshot( std::forward<arg_t>( source ), target );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_SNAPSHOT_HPP
