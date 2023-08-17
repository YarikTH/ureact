//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_PAIRWISE_HPP
#define UREACT_ADAPTOR_PAIRWISE_HPP

#include <ureact/adaptor/adjacent.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Special case of adjacent for 2 events
 * 
 * Emits a tuple with last 2 events starting from the second event
 * 
 *  Semantically equivalent of ranges::pairwise
 */
inline constexpr auto pairwise = adjacent<2>;

UREACT_END_NAMESPACE

#endif //UREACT_ADAPTOR_PAIRWISE_HPP
