#pragma once

#include "doctest_extra.h"
#include "ureact/ureact.hpp"

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
