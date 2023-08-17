//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_PAIRWISE_FILTER_HPP
#define UREACT_ADAPTOR_PAIRWISE_FILTER_HPP

#include <ureact/adaptor/adjacent_filter.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Special case of adjacent_filter for 2 events
 * 
 * After first event, emits new event only if a func result with last 2 events return true
 * 
 *  Semantically equivalent of ranges::pairwise_filter
 */
inline constexpr auto pairwise_filter = adjacent_filter<2>;

UREACT_END_NAMESPACE

#endif //UREACT_ADAPTOR_PAIRWISE_FILTER_HPP
