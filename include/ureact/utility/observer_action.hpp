//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_UTILITY_OBSERVER_ACTION_HPP
#define UREACT_UTILITY_OBSERVER_ACTION_HPP

#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Observer functions can return values of this type to control further processing.
 */
enum class observer_action
{
    next,           ///< Continue observing
    stop_and_detach ///< Stop observing
};

UREACT_END_NAMESPACE

#endif //UREACT_UTILITY_OBSERVER_ACTION_HPP
