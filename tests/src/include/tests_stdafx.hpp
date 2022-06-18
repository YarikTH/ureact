#pragma once

#include <sstream>
#include <string>
#include <tuple>
#include <vector>

#include <doctest.h>

#include "ureact/ureact.hpp"

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

template <class T, class Allocator>
doctest::String toString( const std::vector<T, Allocator>& value )
{
    std::ostringstream ss;

    ss << "[";
    for( auto it = value.begin(), ite = value.end(); it != ite; ++it )
    {
        ss << *it;
        if( it + 1 != ite )
        {
            ss << ", ";
        }
    }
    ss << "]";

    return ss.str().c_str();
}

} // namespace std

const auto always_true = []( auto&& ) { return true; };

// use fold expression to collect all received events into a vector
template <class E>
auto make_collector( const ureact::events<E>& events )
{
    const auto collector = []( std::vector<E>& accum, const E& e ) { accum.push_back( e ); };
    return ureact::fold( events, std::vector<E>{}, collector );
}

// use fold expression to count received events
template <class E>
auto make_counter( const ureact::events<E>& events )
{
    const auto counter = []( size_t& accum, const E& ) { ++accum; };
    return ureact::fold( events, size_t{ 0 }, counter );
}

template <class T>
auto make_deeper( T&& signal )
{
    ureact::signal<int> result = signal;
    for( int i = 0; i < 100; ++i )
        result = +result;
    return result;
}
