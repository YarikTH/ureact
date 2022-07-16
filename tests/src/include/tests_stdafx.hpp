#pragma once

#include <list>
#include <set>
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

template <class Key, class Compare = std::less<Key>, class Allocator = std::allocator<Key>>
doctest::String toString( const std::set<Key, Compare, Allocator>& value )
{
    return containerToString( value );
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

template <class T>
auto make_deeper( T&& signal )
{
    ureact::signal<int> result = signal;
    for( int i = 0; i < 100; ++i )
        result = +result;
    return result;
}

struct copy_stats
{
    int copy_count = 0;
    int move_count = 0;
};

struct copy_counter
{
    int v = 0;
    copy_stats* stats = nullptr;

    copy_counter() = default;

    copy_counter( int x, copy_stats* s )
        : v( x )
        , stats( s )
    {}

    copy_counter( const copy_counter& other )
        : v( other.v )
        , stats( other.stats )
    {
        stats->copy_count++;
    }

    copy_counter( copy_counter&& other ) noexcept
        : v( other.v )
        , stats( other.stats )
    {
        stats->move_count++;
    }

    copy_counter& operator=( const copy_counter& other )
    {
        if( &other != this )
        {
            v = other.v;
            stats = other.stats;
            stats->copy_count++;
        }
        return *this;
    }

    copy_counter& operator=( copy_counter&& other ) noexcept
    {
        if( &other != this )
        {
            v = other.v;
            stats = other.stats;
            stats->move_count++;
        }
        return *this;
    }

    copy_counter operator+( const copy_counter& r ) const
    {
        return copy_counter{ v + r.v, stats };
    }

    bool operator==( const copy_counter& other ) const
    {
        return v == other.v;
    }
};
