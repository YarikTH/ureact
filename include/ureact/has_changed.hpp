//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_HAS_CHANGED_HPP
#define UREACT_HAS_CHANGED_HPP

#include <functional>
#include <type_traits>

#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wfloat-equal"
#endif

/*!
 * @brief has_changed overload for arithmetic types and enums
 *
 * @note Can't use ADL, so should be placed before has_changed_comparable
 * @note float-equal warning is suppressed, because it is perfectly fine to compare them in this context
 */
template <class T, class = std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>>
UREACT_WARN_UNUSED_RESULT constexpr bool has_changed( const T lhs, const T rhs ) noexcept
{
    return !( lhs == rhs );
}

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic pop
#endif

namespace detail
{

template <typename T, typename = void>
struct has_changed_comparable : std::false_type
{};

template <typename T>
struct has_changed_comparable<T,
    std::void_t<decltype( has_changed( std::declval<T>(), std::declval<T>() ) )>> : std::true_type
{};

template <typename T>
inline constexpr bool has_changed_comparable_v = has_changed_comparable<T>::value;

struct HasChangedCPO
{
    template <class T>
    constexpr bool operator()( const T& lhs, const T& rhs ) const noexcept
    {
        if constexpr( has_changed_comparable_v<T> )
            return has_changed( lhs, rhs );
        else
            return true; // Assume always changed
    }
};

/*!
 * @brief std::not_equal_to analog intended to prevent reaction of signals to setting the same value as before aka "calming"
 * 
 * @note To make a user-defined type "calmable", provide correct has_changed function
 *       for the type in the same namespace the type is defined.
 *       Expected signature is "bool has_changed( const T& lhs, const T& rhs )"
 */
inline constexpr detail::HasChangedCPO has_changed{};

} // namespace detail

UREACT_END_NAMESPACE

#endif //UREACT_HAS_CHANGED_HPP
