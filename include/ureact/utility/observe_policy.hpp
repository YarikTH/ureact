//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_UTILITY_OBSERVE_POLICY_HPP
#define UREACT_UTILITY_OBSERVE_POLICY_HPP

#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Signal observer functions can receive values of this type to control initial action.
 */
enum class observe_policy
{
    skip_current,   ///< default behavior. Notify only when signal value has changed
    notify_current, ///< notify current value, then when signal value has changed
};

UREACT_END_NAMESPACE

#endif //UREACT_UTILITY_OBSERVE_POLICY_HPP
