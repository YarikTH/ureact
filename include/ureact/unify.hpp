//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_UNIFY_HPP
#define UREACT_UNIFY_HPP

#include <ureact/cast.hpp>
#include <ureact/unit.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Utility function to transform any event stream into a unit stream
 *
 *  Emits a unit for any event that passes source
 */
template <typename E>
UREACT_WARN_UNUSED_RESULT auto unify( const events<E>& source )
{
    return cast<unit>( source );
}

/*!
 * @brief Curried version of unify(events_t&& source)
 */
//UREACT_WARN_UNUSED_RESULT inline auto unify()
//{
//    return cast<unit>();
//}

UREACT_END_NAMESPACE

#endif // UREACT_UNIFY_HPP
