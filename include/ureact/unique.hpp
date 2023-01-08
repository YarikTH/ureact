//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_UNIQUE_HPP
#define UREACT_UNIQUE_HPP

#include <ureact/closure.hpp>
#include <ureact/filter.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Filter out all except the first element from every consecutive group of equivalent elements
 *
 *  In other words: removes consecutive (adjacent) duplicates
 *
 *  Semantically equivalent of std::unique
 */
template <typename E>
UREACT_WARN_UNUSED_RESULT inline auto unique( const events<E>& source )
{
    return filter( source, [first = true, prev = E{}]( const E& e ) mutable {
        const bool pass = first || e != prev;
        first = false;
        prev = e;
        return pass;
    } );
}

/*!
 * @brief Curried version of unique(const events<E>& source)
 */
UREACT_WARN_UNUSED_RESULT inline auto unique()
{
    return closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return unique( std::forward<arg_t>( source ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_UNIQUE_HPP
