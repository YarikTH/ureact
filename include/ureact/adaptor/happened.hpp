//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_HAPPENED_HPP
#define UREACT_ADAPTOR_HAPPENED_HPP

#include <ureact/adaptor/fold.hpp>
#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Detects if event happened at least once and stores this fact as a signal<bool>
 */
inline constexpr auto happened = fold( false, []( const auto&, bool ) { return true; } );

UREACT_END_NAMESPACE

#endif //UREACT_ADAPTOR_HAPPENED_HPP
