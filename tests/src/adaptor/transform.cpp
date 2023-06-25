//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/transform.hpp"

#include <algorithm>

#include "catch2_extra.hpp"
#include "identity.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/transaction.hpp"

// squaring integer events
TEST_CASE( "ureact::transform" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> squared;
    const auto square = []( auto i ) { return i * i; };

    SECTION( "Functional syntax" )
    {
        squared = ureact::transform( src, square );
    }
    SECTION( "Piped syntax" )
    {
        squared = src | ureact::transform( square );
    }

    const auto result = ureact::collect<std::vector>( squared );

    for( int i = 0; i < 5; ++i )
        src << i;

    const std::vector<int> expected = { 0, 1, 4, 9, 16 };
    CHECK( result.get() == expected );
}

// clamp integer events with limit which values are in range which is presented in the form
// of signals
TEST_CASE( "ureact::transform (synced)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto limit_min = ureact::make_var( ctx, 4 );
    auto limit_max = ureact::make_var( ctx, 7 );

    ureact::events<int> clamped;
    const auto clamp = []( auto i, int min, int max ) { return std::clamp( i, min, max ); };

    SECTION( "Functional syntax" )
    {
        clamped = ureact::transform( src, with( limit_min, limit_max ), clamp );
    }
    SECTION( "Piped syntax" )
    {
        clamped = src | ureact::transform( with( limit_min, limit_max ), clamp );
    }

    const auto result = ureact::collect<std::vector>( clamped );

    for( int i : { -1, 4, 10, 0, 5, 2 } )
        src << i;

    // change limits and pass the same values second time
    do_transaction( ctx, [&]() {
        for( int i : { -1, 4, 10, 0, 5, 2 } )
            src << i;

        limit_min <<= 1;
        limit_max <<= 3;
    } );

    // we expect only numbers in [limit_min, limit_max] range passed our filter
    // synced filtering performed only after new limit values are calculated
    const std::vector<int> expected
        = { /*first range*/ 4, 4, 7, 4, 5, 4, /*second range*/ 1, 3, 3, 1, 3, 2 };
    CHECK( result.get() == expected );
}

// Check how it ureact::transform works with functions that return T&& or T&
TEST_CASE( "ureact::transform (ref)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> rvalue;
    ureact::events<int> lvalue;

    SECTION( "Functional syntax" )
    {
        rvalue = ureact::transform( src, identity{} );
        lvalue = ureact::transform( src, identity_ref{} );
    }
    SECTION( "Piped syntax" )
    {
        rvalue = src | ureact::transform( identity{} );
        lvalue = src | ureact::transform( identity_ref{} );
    }

    const auto result_rvalue = ureact::collect<std::vector>( rvalue );
    const auto result_lvalue = ureact::collect<std::vector>( lvalue );

    for( int i = 0; i < 5; ++i )
        src << i;

    const std::vector<int> expected = { 0, 1, 2, 3, 4 };
    CHECK( result_rvalue.get() == expected );
    CHECK( result_lvalue.get() == expected );
}

TEST_CASE( "ureact::transform_as" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int64_t> big_int;

    SECTION( "Functional syntax" )
    {
        big_int = ureact::transform_as<int64_t>( src, identity{} );
    }
    SECTION( "Piped syntax" )
    {
        big_int = src | ureact::transform_as<int64_t>( identity{} );
    }

    const auto result = ureact::collect<std::vector>( big_int );

    for( int i = 0; i < 5; ++i )
        src << i;

    const std::vector<int64_t> expected = { 0, 1, 2, 3, 4 };
    CHECK( result.get() == expected );
}
