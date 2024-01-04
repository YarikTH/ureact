//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <tuple>
#include <utility>
#include <variant>

#include <catch2/catch_test_macros.hpp>

// Defines are taken from doctest v2.4.11, with prefix changed from DOCTEST_ to UREACT_
// https://github.com/doctest/doctest/blob/v2.4.11/doctest/doctest.h
//
// Copyright (c) 2016-2023 Viktor Kirilov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT

// =================================================================================================
// == COMPILER VERSION =============================================================================
// =================================================================================================

// ideas for the version stuff are taken from here: https://github.com/cxxstuff/cxx_detect

#ifdef _MSC_VER
#    define UREACT_CPLUSPLUS _MSVC_LANG
#else
#    define UREACT_CPLUSPLUS __cplusplus
#endif

#define UREACT_COMPILER( MAJOR, MINOR, PATCH ) ( (MAJOR)*10000000 + (MINOR)*100000 + ( PATCH ) )

// GCC/Clang and GCC/MSVC are mutually exclusive, but Clang/MSVC are not because of clang-cl...
#if defined( _MSC_VER ) && defined( _MSC_FULL_VER )
#    if _MSC_VER == _MSC_FULL_VER / 10000
#        define UREACT_MSVC UREACT_COMPILER( _MSC_VER / 100, _MSC_VER % 100, _MSC_FULL_VER % 10000 )
#    else // MSVC
#        define UREACT_MSVC                                                                        \
            UREACT_COMPILER(                                                                       \
                _MSC_VER / 100, ( _MSC_FULL_VER / 100000 ) % 100, _MSC_FULL_VER % 100000 )
#    endif // MSVC
#endif     // MSVC
#if defined( __clang__ ) && defined( __clang_minor__ ) && defined( __clang_patchlevel__ )
#    define UREACT_CLANG UREACT_COMPILER( __clang_major__, __clang_minor__, __clang_patchlevel__ )
#elif defined( __GNUC__ ) && defined( __GNUC_MINOR__ ) && defined( __GNUC_PATCHLEVEL__ )           \
    && !defined( __INTEL_COMPILER )
#    define UREACT_GCC UREACT_COMPILER( __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__ )
#endif // GCC
#if defined( __INTEL_COMPILER )
#    define UREACT_ICC UREACT_COMPILER( __INTEL_COMPILER / 100, __INTEL_COMPILER % 100, 0 )
#endif // ICC

#ifndef UREACT_MSVC
#    define UREACT_MSVC 0
#endif // UREACT_MSVC
#ifndef UREACT_CLANG
#    define UREACT_CLANG 0
#endif // UREACT_CLANG
#ifndef UREACT_GCC
#    define UREACT_GCC 0
#endif // UREACT_GCC
#ifndef UREACT_ICC
#    define UREACT_ICC 0
#endif // UREACT_ICC

// =================================================================================================
// == COMPILER WARNINGS HELPERS ====================================================================
// =================================================================================================

#if UREACT_CLANG && !UREACT_ICC
#    define UREACT_PRAGMA_TO_STR( x ) _Pragma( #    x )
#    define UREACT_CLANG_SUPPRESS_WARNING_PUSH _Pragma( "clang diagnostic push" )
#    define UREACT_CLANG_SUPPRESS_WARNING( w ) UREACT_PRAGMA_TO_STR( clang diagnostic ignored w )
#    define UREACT_CLANG_SUPPRESS_WARNING_POP _Pragma( "clang diagnostic pop" )
#    define UREACT_CLANG_SUPPRESS_WARNING_WITH_PUSH( w )                                           \
        UREACT_CLANG_SUPPRESS_WARNING_PUSH UREACT_CLANG_SUPPRESS_WARNING( w )
#else // UREACT_CLANG
#    define UREACT_CLANG_SUPPRESS_WARNING_PUSH
#    define UREACT_CLANG_SUPPRESS_WARNING( w )
#    define UREACT_CLANG_SUPPRESS_WARNING_POP
#    define UREACT_CLANG_SUPPRESS_WARNING_WITH_PUSH( w )
#endif // UREACT_CLANG

#if UREACT_GCC
#    define UREACT_PRAGMA_TO_STR( x ) _Pragma( #    x )
#    define UREACT_GCC_SUPPRESS_WARNING_PUSH _Pragma( "GCC diagnostic push" )
#    define UREACT_GCC_SUPPRESS_WARNING( w ) UREACT_PRAGMA_TO_STR( GCC diagnostic ignored w )
#    define UREACT_GCC_SUPPRESS_WARNING_POP _Pragma( "GCC diagnostic pop" )
#    define UREACT_GCC_SUPPRESS_WARNING_WITH_PUSH( w )                                             \
        UREACT_GCC_SUPPRESS_WARNING_PUSH UREACT_GCC_SUPPRESS_WARNING( w )
#else // UREACT_GCC
#    define UREACT_GCC_SUPPRESS_WARNING_PUSH
#    define UREACT_GCC_SUPPRESS_WARNING( w )
#    define UREACT_GCC_SUPPRESS_WARNING_POP
#    define UREACT_GCC_SUPPRESS_WARNING_WITH_PUSH( w )
#endif // UREACT_GCC

#if UREACT_MSVC
#    define UREACT_MSVC_SUPPRESS_WARNING_PUSH __pragma( warning( push ) )
#    define UREACT_MSVC_SUPPRESS_WARNING( w ) __pragma( warning( disable : w ) )
#    define UREACT_MSVC_SUPPRESS_WARNING_POP __pragma( warning( pop ) )
#    define UREACT_MSVC_SUPPRESS_WARNING_WITH_PUSH( w )                                            \
        UREACT_MSVC_SUPPRESS_WARNING_PUSH UREACT_MSVC_SUPPRESS_WARNING( w )
#else // UREACT_MSVC
#    define UREACT_MSVC_SUPPRESS_WARNING_PUSH
#    define UREACT_MSVC_SUPPRESS_WARNING( w )
#    define UREACT_MSVC_SUPPRESS_WARNING_POP
#    define UREACT_MSVC_SUPPRESS_WARNING_WITH_PUSH( w )
#endif // UREACT_MSVC

namespace std // NOLINT
{

template <size_t n, typename... T>
typename std::enable_if<( n >= sizeof...( T ) )>::type print_tuple(
    std::ostream&, const std::tuple<T...>& )
{}

template <size_t n, typename... T>
typename std::enable_if<( n < sizeof...( T ) )>::type print_tuple(
    std::ostream& os, const std::tuple<T...>& tup )
{
    if constexpr( n != 0 )
        os << ", ";
    os << std::get<n>( tup );
    print_tuple<n + 1>( os, tup );
}

template <typename... T>
std::ostream& operator<<( std::ostream& os, const std::tuple<T...>& tup )
{
    os << "[";
    print_tuple<0>( os, tup );
    return os << "]";
}

template <typename T, typename Y>
std::ostream& operator<<( std::ostream& os, const std::pair<T, Y>& pair )
{
    os << "[";
    os << pair.first;
    os << ", ";
    os << pair.second;
    return os << "]";
}

template <typename T1, typename... T>
std::ostream& operator<<( std::ostream& os, const std::variant<T1, T...>& variant )
{
    std::visit( [&os]( auto&& arg ) { os << arg; }, variant );
    return os;
}

} // namespace std
