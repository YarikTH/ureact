//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_CAST_HPP
#define UREACT_CAST_HPP

#include <ureact/closure.hpp>
#include <ureact/transform.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Create a new event stream that casts events from other stream using static_cast
 *
 *  For every event e in source, emit t = static_cast<OutE>(e).
 *
 *  Type of resulting signal have to be explicitly specified.
 */
template <typename OutE, typename InE>
UREACT_WARN_UNUSED_RESULT auto cast( const events<InE>& source ) -> events<OutE>
{
    return transform( source, //
        []( const InE& e ) {  //
            return static_cast<OutE>( e );
        } );
}

/*!
 * @brief Curried version of cast(const events<InE>& source)
 */
template <typename OutE>
UREACT_WARN_UNUSED_RESULT auto cast()
{
    return closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return cast<OutE>( std::forward<arg_t>( source ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_CAST_HPP
