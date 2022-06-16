//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

namespace
{
struct Stats
{
    int copyCount = 0;
    int moveCount = 0;
};

struct CopyCounter
{
    int v = 0;
    Stats* stats = nullptr;

    CopyCounter() = default;

    CopyCounter( int x, Stats* s )
        : v( x )
        , stats( s )
    {}

    CopyCounter( const CopyCounter& other )
        : v( other.v )
        , stats( other.stats )
    {
        stats->copyCount++;
    }

    CopyCounter( CopyCounter&& other ) noexcept
        : v( other.v )
        , stats( other.stats )
    {
        stats->moveCount++;
    }

    CopyCounter& operator=( const CopyCounter& other )
    {
        if( &other != this )
        {
            v = other.v;
            stats = other.stats;
            stats->copyCount++;
        }
        return *this;
    }

    CopyCounter& operator=( CopyCounter&& other ) noexcept
    {
        if( &other != this )
        {
            v = other.v;
            stats = other.stats;
            stats->moveCount++;
        }
        return *this;
    }

    CopyCounter operator+( const CopyCounter& r ) const
    {
        return CopyCounter{ v + r.v, stats };
    }

    bool operator==( const CopyCounter& other ) const
    {
        return v == other.v;
    }
};
} // namespace

TEST_SUITE_BEGIN( "MoveTest" );

TEST_CASE( "Copy1" )
{
    ureact::context ctx;

    Stats stats1;

    auto a = ureact::make_var( ctx, CopyCounter{ 1, &stats1 } );
    auto b = ureact::make_var( ctx, CopyCounter{ 10, &stats1 } );
    auto c = ureact::make_var( ctx, CopyCounter{ 100, &stats1 } );
    auto d = ureact::make_var( ctx, CopyCounter{ 1000, &stats1 } );

    // 4x move to m_value_
    // 4x copy to m_new_value (can't be uninitialized for references)
    CHECK( stats1.copyCount == 4 );
    CHECK( stats1.moveCount == 4 );

    auto x = a + b + c + d;

    CHECK( stats1.copyCount == 4 );
    CHECK( stats1.moveCount == 7 );
    CHECK( x.get().v == 1111 );

    a <<= CopyCounter{ 2, &stats1 };

    CHECK( stats1.copyCount == 4 );
    CHECK( stats1.moveCount == 10 );
    CHECK( x.get().v == 1112 );
}

TEST_SUITE_END();
