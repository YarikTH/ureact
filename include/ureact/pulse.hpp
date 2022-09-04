//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_PULSE_HPP_
#define UREACT_PULSE_HPP_

#include "process.hpp"

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Emits the value of a target signal when an event is received
 *
 *  Creates an event stream that emits target.get() when receiving an event from trigger.
 *  The values of the received events are irrelevant.
 */
template <typename S, typename E>
UREACT_WARN_UNUSED_RESULT auto pulse( const events<E>& trigger, const signal<S>& target )
    -> events<S>
{
    return process<S>( trigger,
        with( target ),
        []( event_range<E> range, event_emitter<S> out, const S& target_value ) {
            for( size_t i = 0, ie = range.size(); i < ie; ++i )
                out << target_value;
        } );
}

/*!
 * @brief Curried version of pulse() algorithm used for "pipe" syntax
 */
template <typename S>
UREACT_WARN_UNUSED_RESULT auto pulse( const signal<S>& target )
{
    return closure{ [target = target]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return pulse( std::forward<arg_t>( source ), target );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_PULSE_HPP_
