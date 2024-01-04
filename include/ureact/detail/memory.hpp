//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_MEMORY_HPP
#define UREACT_DETAIL_MEMORY_HPP

#include <memory>
#include <new>

#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

// Partial alternative to <memory>, including backported functions
namespace detail
{

// TODO: make macro to eliminate duplication here
#if defined( __cpp_lib_launder ) && __cpp_lib_launder >= 201606L

using std::launder;

#else

template <typename T>
[[nodiscard]] constexpr T* launder( T* p ) noexcept
{
    return p;
}

#endif

// -stdlib=libc++ doesn't have std::construct_at till v12.0.0
#if __cplusplus >= 202002L && ( !defined( _LIBCPP_VERSION ) || _LIBCPP_VERSION >= 12000 )

using std::construct_at;

#else

// Backport from https://en.cppreference.com/w/cpp/memory/construct_at
template <class T, class... Args>
constexpr T* construct_at( T* p, Args&&... args )
{
    return ::new( const_cast<void*>( static_cast<const volatile void*>( p ) ) )
        T( std::forward<Args>( args )... );
}

#endif

// It is added in C++17, assume all major compilers have it
using std::destroy_at;

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_MEMORY_HPP
