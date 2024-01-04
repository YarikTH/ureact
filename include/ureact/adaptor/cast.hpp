//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_CAST_HPP
#define UREACT_ADAPTOR_CAST_HPP

#include <ureact/adaptor/transform.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Create a new event stream that casts events from other stream using static_cast
 *
 *  For every event e in source, emit t = static_cast<OutE>(e).
 *
 *  Type of resulting ureact::events<E> have to be explicitly specified.
 */
template <typename E>
inline constexpr auto cast = transform( []( const auto& e ) { return static_cast<E>( e ); } );

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_CAST_HPP
