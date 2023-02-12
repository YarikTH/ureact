//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_COUNT_HPP
#define UREACT_COUNT_HPP

#include <ureact/fold.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Counts amount of received events into signal<S>
 *
 *  Type of resulting signal should be explicitly specified.
 *  Value type should be default constructing and prefix incremented
 *
 *  @warning Not to be confused with std::count(from, to, value)
 */
template <typename S>
inline constexpr auto count_ = fold( S{}, []( const auto&, S& accum ) { ++accum; } );

/*!
 * @brief Counts amount of received events into signal<size_t>
 *
 *  @warning Not to be confused with std::count(from, to, value)
 */
inline constexpr auto count = count_<size_t>;

UREACT_END_NAMESPACE

#endif // UREACT_COUNT_HPP
