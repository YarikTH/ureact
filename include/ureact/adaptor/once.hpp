//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_ONCE_HPP
#define UREACT_ADAPTOR_ONCE_HPP

#include <ureact/adaptor/take.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Passes a single event
 */
inline constexpr auto once = take( 1 );

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_ONCE_HPP
