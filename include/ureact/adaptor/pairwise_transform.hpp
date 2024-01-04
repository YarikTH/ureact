//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_PAIRWISE_TRANSFORM_HPP
#define UREACT_ADAPTOR_PAIRWISE_TRANSFORM_HPP

#include <ureact/adaptor/adjacent_transform.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Special case of adjacent_transform for 2 events
 * 
 * Emits a func result with last 2 events starting from the second event
 * 
 *  Semantically equivalent of ranges::pairwise_transform
 */
inline constexpr auto pairwise_transform = adjacent_transform<2>;

UREACT_END_NAMESPACE

#endif //UREACT_ADAPTOR_PAIRWISE_TRANSFORM_HPP
