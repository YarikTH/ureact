//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/flatten.hpp"

#include <algorithm>
#include <queue>

#include "catch2_extra.hpp"
#include "ureact/adaptor/lift.hpp"
#include "ureact/adaptor/observe.hpp"
#include "ureact/events.hpp"
#include "ureact/signal.hpp"
#include "ureact/transaction.hpp"

// TODO: replace FlattenN tests with more meaningful ones

TEST_CASE( "Flatten1" )
{
    ureact::context ctx;

    auto inner1 = make_var( ctx, 123 );
    auto inner2 = make_var( ctx, 789 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = ureact::flatten( outer );

    std::queue<int> results;

    ureact::observer _ = ureact::observe( flattened, [&]( int v ) { results.push( v ); } );

    CHECK( outer.get().equal_to( inner1 ) );
    CHECK( flattened.get() == 123 );

    inner1 <<= 456;

    CHECK( flattened.get() == 456 );

    CHECK( results.front() == 456 );
    results.pop();
    CHECK( results.empty() );

    outer <<= inner2;

    CHECK( outer.get().equal_to( inner2 ) );
    CHECK( flattened.get() == 789 );

    CHECK( results.front() == 789 );
    results.pop();
    CHECK( results.empty() );
}

TEST_CASE( "Flatten2" )
{
    ureact::context ctx;

    auto a0 = make_var( ctx, 100 );

    auto inner1 = make_var( ctx, 200 );

    auto a1 = make_var( ctx, 300 );
    auto a2 = a1 + 0;
    auto a3 = a2 + 0;
    auto a4 = a3 + 0;
    auto a5 = a4 + 0;
    auto a6 = a5 + 0;
    auto inner2 = a6 + 0;

    CHECK( inner1.get() == 200 );
    CHECK( inner2.get() == 300 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = ureact::flatten( outer );

    CHECK( flattened.get() == 200 );

    int observeCount = 0;

    ureact::observer obs
        = ureact::observe( flattened, [&observeCount]( int /*unused*/ ) { observeCount++; } );

    auto o1 = a0 + flattened;
    auto o2 = o1 + 0;
    auto o3 = o2 + 0;
    auto result = o3 + 0;

    CHECK( result.get() == 100 + 200 );

    inner1 <<= 1234;

    CHECK( result.get() == 100 + 1234 );
    CHECK( observeCount == 1 );

    outer <<= inner2;

    CHECK( result.get() == 100 + 300 );
    CHECK( observeCount == 2 );

    {
        ureact::transaction _{ ctx };
        a0 <<= 5000;
        a1 <<= 6000;
    }

    CHECK( result.get() == 5000 + 6000 );
    CHECK( observeCount == 3 );
}

TEST_CASE( "Flatten3" )
{
    ureact::context ctx;

    auto inner1 = make_var( ctx, 10 );

    auto a1 = make_var( ctx, 20 );
    auto a2 = a1 + 0;
    auto a3 = a2 + 0;
    auto inner2 = a3 + 0;

    auto outer = make_var( ctx, inner1 );

    auto a0 = make_var( ctx, 30 );

    auto flattened = ureact::flatten( outer );

    int observeCount = 0;

    ureact::observer obs
        = ureact::observe( flattened, [&observeCount]( int /*unused*/ ) { observeCount++; } );

    auto result = flattened + a0;

    CHECK( result.get() == 10 + 30 );
    CHECK( observeCount == 0 );

    {
        ureact::transaction _{ ctx };
        inner1 <<= 1000;
        a0 <<= 200000;
        a1 <<= 50000;
        outer <<= inner2;
    }

    CHECK( result.get() == 50000 + 200000 );
    CHECK( observeCount == 1 );

    {
        ureact::transaction _{ ctx };
        a0 <<= 667;
        a1 <<= 776;
    }

    CHECK( result.get() == 776 + 667 );
    CHECK( observeCount == 2 );

    {
        ureact::transaction _{ ctx };
        inner1 <<= 999;
        a0 <<= 888;
    }

    CHECK( result.get() == 776 + 888 );
    CHECK( observeCount == 2 );
}

TEST_CASE( "Flatten4" )
{
    ureact::context ctx;

    std::vector<int> results;

    auto a1 = make_var( ctx, 100 );
    auto inner1 = a1 + 0;

    auto a2 = make_var( ctx, 200 );
    auto inner2 = a2;

    auto a3 = make_var( ctx, 200 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = ureact::flatten( outer );

    auto result = flattened + a3;

    ureact::observer obs = ureact::observe( result, [&]( int v ) { results.push_back( v ); } );

    {
        ureact::transaction _{ ctx };
        a3 <<= 400;
        outer <<= inner2;
    }

    CHECK( results.size() == 1 );

    const bool valueFound = std::find( results.begin(), results.end(), 600 ) != results.end();
    CHECK( valueFound );
}

TEST_CASE( "Flatten5" )
{
    ureact::context ctx;

    auto inner1 = make_var( ctx, 123 );
    auto inner2 = make_var( ctx, 123 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = ureact::flatten( outer );

    std::queue<int> results;

    ureact::observer _ = ureact::observe( flattened, [&]( int v ) { results.push( v ); } );

    CHECK( outer.get().equal_to( inner1 ) );
    CHECK( flattened.get() == 123 );

    CHECK( results.empty() );

    outer <<= inner2;

    CHECK( outer.get().equal_to( inner2 ) );
    CHECK( flattened.get() == 123 );

    // flattened observer shouldn't trigger if value isn't changed
    CHECK( results.empty() );
}

TEST_CASE( "Member1" )
{
    ureact::context ctx;

    auto outer = make_var( ctx, 10 );
    auto inner = make_var( ctx, outer );

    auto flattened = ureact::flatten( inner );

    ureact::observer _ = ureact::observe( flattened, []( int v ) { CHECK( v == 30 ); } );

    outer <<= 30;
}

TEST_CASE( "Signal of events" )
{
    ureact::context ctx;

    auto in1 = ureact::make_source<int>( ctx );
    auto in2 = ureact::make_source<int>( ctx );

    auto sig = make_var( ctx, in1 );

    int reassign_count = 0;

    ureact::observer obs1
        = ureact::observe( sig, [&]( const ureact::events<int>& ) { ++reassign_count; } );

    ureact::events<int> e;

    SECTION( "Functional syntax" )
    {
        e = ureact::flatten( sig );
    }
    SECTION( "Piped syntax" )
    {
        e = sig | ureact::flatten;
    }

    std::vector<int> saved_events;

    ureact::observer obs2
        = ureact::observe( e, [&]( const int value ) { saved_events.push_back( value ); } );

    in1 << -1;
    in2 << 1;

    sig <<= in2;

    in1 << -2;
    in2 << 2;

    CHECK( saved_events == std::vector<int>{ -1, 2 } );
    CHECK( reassign_count == 1 );
}
