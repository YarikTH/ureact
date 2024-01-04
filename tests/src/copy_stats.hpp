//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

struct copy_stats
{
    int copy_count = 0;
    int move_count = 0;
};

// TODO: template type of included value
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
