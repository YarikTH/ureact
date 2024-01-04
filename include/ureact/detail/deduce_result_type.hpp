//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_DEDUCE_RESULT_TYPE_HPP
#define UREACT_DETAIL_DEDUCE_RESULT_TYPE_HPP

#include <type_traits>

#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

/// Deduce decayed function result unless RequestedType is specified
/// TODO: consider adding strip_reference_wrapper: std::reference_wrapper<T> -> T&
template <typename RequestedType, typename F, typename... Args>
using deduce_result_type = std::conditional_t< //
    std::is_same_v<RequestedType, void>,       //
    std::decay_t<std::invoke_result_t<F, Args...>>,
    RequestedType>;

} // namespace detail

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_DEDUCE_RESULT_TYPE_HPP
