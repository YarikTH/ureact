//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/filter.hpp"

#include "catch2_extra.hpp"
#include "identity.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/transaction.hpp"

// filter only even integer events
// our check function depends only on event value
TEST_CASE( "ureact::filter" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> filtered;
    const auto is_even = []( auto i ) { return i % 2 == 0; };

    SECTION( "Functional syntax" )
    {
        filtered = ureact::filter( src, is_even );
    }
    SECTION( "Piped syntax" )
    {
        filtered = src | ureact::filter( is_even );
    }

    const auto result = ureact::collect<std::vector>( filtered );

    for( int i = 0; i < 10; ++i )
        src << i;

    // we expect only even numbers passed our filter
    const std::vector<int> expected = { 0, 2, 4, 6, 8 };
    CHECK( result.get() == expected );
}

// filter integer events which values are in range which is presented in the form
// of signals
TEST_CASE( "ureact::filter (synced)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto limit_min = ureact::make_var( ctx, 4 );
    auto limit_max = ureact::make_var( ctx, 7 );

    ureact::events<int> filtered;
    const auto in_range = []( auto i, int min, int max ) { return i >= min && i <= max; };

    SECTION( "Functional syntax" )
    {
        filtered = ureact::filter( src, with( limit_min, limit_max ), in_range );
    }
    SECTION( "Piped syntax" )
    {
        filtered = src | ureact::filter( with( limit_min, limit_max ), in_range );
    }

    const auto result = ureact::collect<std::vector>( filtered );

    for( int i = 0; i < 10; ++i )
        src << i;

    // change limits and pass the same values second time
    do_transaction( ctx, [&]() {
        for( int i = 0; i < 10; ++i )
            src << i;

        limit_min <<= 1;
        limit_max <<= 3;
    } );

    // we expect only numbers in [limit_min, limit_max] range passed our filter
    // synced filtering performed only after new limit values are calculated
    const std::vector<int> expected = { /*first range*/ 4, 5, 6, 7, /*second range*/ 1, 2, 3 };
    CHECK( result.get() == expected );
}

TEST_CASE( "ureact::filter (ref)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> rvalue;
    ureact::events<int> lvalue;

    SECTION( "Functional syntax" )
    {
        rvalue = ureact::filter( src, identity{} );
        lvalue = ureact::filter( src, identity_ref{} );
    }
    SECTION( "Piped syntax" )
    {
        rvalue = src | ureact::filter( identity{} );
        lvalue = src | ureact::filter( identity_ref{} );
    }

    const auto result_rvalue = ureact::collect<std::vector>( rvalue );
    const auto result_lvalue = ureact::collect<std::vector>( lvalue );

    for( int i : { 1, 0, -1, 0, 0, 2 } )
        src << i;

    const std::vector<int> expected = { 1, -1, 2 };
    CHECK( result_rvalue.get() == expected );
    CHECK( result_lvalue.get() == expected );
}
