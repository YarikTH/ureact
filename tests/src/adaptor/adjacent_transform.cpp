//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/adjacent_transform.hpp"

#include <algorithm>

#include "catch2_extra.hpp"
#include "identity.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"
#include "ureact/transaction.hpp"

TEST_CASE( "ureact::adjacent_transform" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> s;
    ureact::events<std::tuple<int, int, int>> t;

    const auto sum_all = []( auto... ints ) { return ( ... + ints ); };
    const auto make_tuple = []( auto... ints ) { return std::make_tuple( ints... ); };

    SECTION( "Functional syntax" )
    {
        s = ureact::adjacent_transform<3>( src, sum_all );
        t = ureact::adjacent_transform<3>( src, make_tuple );
    }
    SECTION( "Piped syntax" )
    {
        s = src | ureact::adjacent_transform<3>( sum_all );
        t = src | ureact::adjacent_transform<3>( make_tuple );
    }

    const auto result_sum = ureact::collect<std::vector>( s );
    const auto result_tuple = ureact::collect<std::vector>( t );

    for( int i : { 1, 2, 3, 4, 5, 6 } )
        src << i;

    const std::vector<int> expected_sum = {
        /*            */ 6,
        /*               */ 9,
        /*                  */ 12,
        /*                     */ 15,
    };

    const std::vector<std::tuple<int, int, int>> expected_tuple = {
        /*    */ { 1, 2, 3 },
        /*       */ { 2, 3, 4 },
        /*          */ { 3, 4, 5 },
        /*             */ { 4, 5, 6 },
    };

    CHECK( result_sum.get() == expected_sum );
    CHECK( result_tuple.get() == expected_tuple );
}

// clamp integer events with limit which values are in range which is presented in the form
// of signals
TEST_CASE( "ureact::adjacent_transform (synced)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto limit_min = ureact::make_var( ctx, 7 );
    auto limit_max = ureact::make_var( ctx, 13 );

    ureact::events<int> clamped;
    const auto clamped_sum_of_three = []( int x1, int x2, int x3, int min, int max ) {
        return std::clamp( x1 + x2 + x3, min, max );
    };

    SECTION( "Functional syntax" )
    {
        clamped = ureact::adjacent_transform<3>(
            src, with( limit_min, limit_max ), clamped_sum_of_three );
    }
    SECTION( "Piped syntax" )
    {
        clamped
            = src
            | ureact::adjacent_transform<3>( with( limit_min, limit_max ), clamped_sum_of_three );
    }

    const auto result = ureact::collect<std::vector>( clamped );

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
    // synced adjacent_transform performed only after new limit values are calculated
    const std::vector<int> expected
        = { /*first range*/ 7, 9, 12, 13, /*second range*/ 20, 21, 24, 25 };
    CHECK( result.get() == expected );
}

TEST_CASE( "ureact::adjacent_transform (ref)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> rvalue;
    ureact::events<int> lvalue;

    SECTION( "Functional syntax" )
    {
        rvalue = ureact::adjacent_transform<1>( src, identity{} );
        lvalue = ureact::adjacent_transform<1>( src, identity_ref{} );
    }
    SECTION( "Piped syntax" )
    {
        rvalue = src | ureact::adjacent_transform<1>( identity{} );
        lvalue = src | ureact::adjacent_transform<1>( identity_ref{} );
    }

    const auto result_rvalue = ureact::collect<std::vector>( rvalue );
    const auto result_lvalue = ureact::collect<std::vector>( lvalue );

    for( int i = 0; i < 5; ++i )
        src << i;

    const std::vector<int> expected = { 0, 1, 2, 3, 4 };
    CHECK( result_rvalue.get() == expected );
    CHECK( result_lvalue.get() == expected );
}
