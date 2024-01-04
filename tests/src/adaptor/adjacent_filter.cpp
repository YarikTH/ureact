//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/adjacent_filter.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/drop.hpp"
#include "ureact/events.hpp"
#include "ureact/transaction.hpp"

TEST_CASE( "ureact::adjacent_filter" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> s;

    const auto greater_diff
        = []( int a, int b, int value ) { return std::abs( value - b ) > std::abs( a - b ); };

    SECTION( "Functional syntax" )
    {
        s = ureact::adjacent_filter<3>( src, greater_diff );
    }
    SECTION( "Piped syntax" )
    {
        s = src | ureact::adjacent_filter<3>( greater_diff );
    }

    const auto result = ureact::collect<std::vector>( s );

    for( int i : { 1, 2, 3, 5, 5, 6 } )
        src << i;
    //                1  1  2  0  1

    const std::vector<int> expected = {
        /*      */ 1,
        /*         */ 2,
        /*               */ 5,
        /*                     */ 6,
    };

    CHECK( result.get() == expected );
}

// clamp integer events with limit which values are in range which is presented in the form
// of signals
TEST_CASE( "ureact::adjacent_filter (synced)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto limit_min = ureact::make_var( ctx, 7 );
    auto limit_max = ureact::make_var( ctx, 13 );

    ureact::events<int> clamped;
    const auto sum_of_three_in_range = []( int x1, int x2, int x3, int min, int max ) {
        const auto sum = x1 + x2 + x3;
        return min <= sum && sum <= max;
    };

    SECTION( "Functional syntax" )
    {
        clamped = ureact::adjacent_filter<3>(
            src, with( limit_min, limit_max ), sum_of_three_in_range );
    }
    SECTION( "Piped syntax" )
    {
        clamped = src
                | ureact::adjacent_filter<3>( with( limit_min, limit_max ), sum_of_three_in_range );
    }

    const auto result = ureact::collect<std::vector>( clamped | ureact::drop( 2 ) );

    for( int i : { 1, 2, 3, 4, 5, 6 } )
        src << i;

    // change limits and pass next values
    {
        ureact::transaction _{ ctx };

        for( int i : { 7, 8, 9, 10 } )
            src << i;

        limit_min <<= 20;
        limit_max <<= 25;
    }

    // we expect only numbers in [limit_min, limit_max] range passed our filter
    // synced adjacent_filter performed only after new limit values are calculated
    const std::vector<int> expected = { /*first range*/ 4, 5, /*second range*/ 8, 9 };
    CHECK( result.get() == expected );
}
