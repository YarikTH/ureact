//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
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
    if( n != 0 )
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
