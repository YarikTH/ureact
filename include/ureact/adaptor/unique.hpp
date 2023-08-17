//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_UNIQUE_HPP
#define UREACT_ADAPTOR_UNIQUE_HPP

#include <ureact/adaptor/pairwise_filter.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Filter out all except the first element from every consecutive group of equivalent elements
 *
 *  In other words: removes consecutive (adjacent) duplicates
 *
 *  Semantically equivalent of std::unique
 */
inline constexpr auto unique = pairwise_filter(
    []( const auto& last, const auto& current ) { return !( current == last ); } );

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_UNIQUE_HPP
