//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_VALUES_HPP
#define UREACT_ADAPTOR_VALUES_HPP

#include <ureact/adaptor/elements.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Takes event stream of tuple-like values and creates a new event stream with a value-type of the second element of received value-type
 * 
 *  For every event e in source, emit t = std::get<1>(e).
 */
inline constexpr auto values = elements<1>;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_VALUES_HPP
