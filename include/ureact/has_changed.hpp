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

template <typename S>
class signal;

template <typename E>
class events;

namespace detail
{

/// c++17 analog of equality_comparable concept from c++20
/// https://en.cppreference.com/w/cpp/concepts/equality_comparable
template <typename T, typename = void>
struct equality_comparable : std::false_type
{};

// TODO: check if result of == is exactly bool
template <typename T>
struct equality_comparable<T, std::void_t<decltype( std::declval<T>() == std::declval<T>() )>>
    : std::true_type
{};

template <typename T>
inline constexpr bool equality_comparable_v = equality_comparable<T>::value;

} // namespace detail

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wfloat-equal"
#endif

/*!
 * @brief std::not_equal_to analog intended to prevent reaction of signals to setting the same value as before aka "calming"
 *
 *  Additionally:
 *  * it equally compares signal<S> and events<E> even if their operator== is overloaded
 *  * it equally compares reference wrappers because they can be used as S for signal<S> and their operator== does unexpected compare
 *  * it returns true if types are not equally comparable otherwise
 */
template <typename T>
UREACT_WARN_UNUSED_RESULT constexpr bool has_changed( const T& lhs, const T& rhs )
{
    if constexpr( detail::equality_comparable_v<T> )
    {
        return !( lhs == rhs );
    }
    else
    {
        return true;
    }
}

// TODO: check if lhs.equal_to( rhs ) can be called instead of checking for specific types
template <typename S>
UREACT_WARN_UNUSED_RESULT constexpr bool has_changed( const signal<S>& lhs, const signal<S>& rhs )
{
    return !lhs.equal_to( rhs );
}

template <typename E>
UREACT_WARN_UNUSED_RESULT constexpr bool has_changed( const events<E>& lhs, const events<E>& rhs )
{
    return !lhs.equal_to( rhs );
}

template <typename T>
UREACT_WARN_UNUSED_RESULT constexpr bool has_changed( //
    const std::reference_wrapper<T>& lhs,             //
    const std::reference_wrapper<T>& rhs )
{
    return has_changed( lhs.get(), rhs.get() );
}

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic pop
#endif

UREACT_END_NAMESPACE

#endif //UREACT_HAS_CHANGED_HPP
