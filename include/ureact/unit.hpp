//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_UNIT_HPP
#define UREACT_UNIT_HPP

#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief This class is used as value type of unit streams, which emit events without any value other than the fact that they occurred
 *
 *  See std::monostate https://en.cppreference.com/w/cpp/utility/variant/monostate
 *  See Regular Void https://open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0146r1.html#ThinkingAboutVoid
 */
struct unit
{
    constexpr unit() = default;
    constexpr unit( const unit& ) = default;
    constexpr unit& operator=( const unit& ) = default;

    // unit can be constructed from any value
    template <class T>
    explicit constexpr unit( T&& ) noexcept // NOLINT(bugprone-forwarding-reference-overload)
    {}
};

// clang-format off
constexpr bool operator==(unit, unit) noexcept { return true; }
constexpr bool operator!=(unit, unit) noexcept { return false; }
constexpr bool operator< (unit, unit) noexcept { return false; }
constexpr bool operator> (unit, unit) noexcept { return false; }
constexpr bool operator<=(unit, unit) noexcept { return true; }
constexpr bool operator>=(unit, unit) noexcept { return true; }
//constexpr std::strong_ordering operator<=>(unit, unit) noexcept { return std::strong_ordering::equal; }
// clang-format on

UREACT_END_NAMESPACE

#endif //UREACT_UNIT_HPP
