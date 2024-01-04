//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_CHANGED_HPP
#define UREACT_ADAPTOR_CHANGED_HPP

#include <ureact/adaptor/monitor.hpp>
#include <ureact/adaptor/unify.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Emits unit when target signal was changed
 *
 *  Creates a unit stream that emits when target is changed.
 */
inline constexpr auto changed = monitor | unify;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_CHANGED_HPP
