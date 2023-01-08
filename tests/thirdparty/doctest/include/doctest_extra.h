//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef UREACT_DOCTEST_EXTRA_H
#define UREACT_DOCTEST_EXTRA_H

#include <doctest.h>
#include <tuple>
#include <sstream>
#include <vector>
#include <deque>
#include <list>
#include <set>

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

template <class T>
doctest::String containerToString( const T& value )
{
    std::ostringstream ss;

    ss << "[";
    for( auto it = value.begin(), ite = value.end(); it != ite; ++it )
    {
        ss << *it;
        if( std::next( it ) != ite )
        {
            ss << ", ";
        }
    }
    ss << "]";

    return ss.str().c_str();
}

template <class T, class Allocator>
doctest::String toString( const std::vector<T, Allocator>& value )
{
    return containerToString( value );
}

template <class T, class Allocator>
doctest::String toString( const std::deque<T, Allocator>& value )
{
    return containerToString( value );
}

template <class T, class Allocator>
doctest::String toString( const std::list<T, Allocator>& value )
{
    return containerToString( value );
}

template <class Key, class Compare, class Allocator>
doctest::String toString( const std::set<Key, Compare, Allocator>& value )
{
    return containerToString( value );
}

} // namespace std

#endif //UREACT_DOCTEST_EXTRA_H
