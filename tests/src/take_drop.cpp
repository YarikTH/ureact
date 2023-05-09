//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/drop.hpp"
#include "ureact/adaptor/once.hpp"
#include "ureact/adaptor/slice.hpp"
#include "ureact/adaptor/take.hpp"

// filters that take first N elements or skip first N elements
TEST_CASE( "TakeDropSlice" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> first_n;
    ureact::events<int> without_first_n;
    ureact::events<int> middle;

    SECTION( "Functional syntax" )
    {
        first_n = ureact::take( src, 5 );
        without_first_n = ureact::drop( src, 5 );
        middle = ureact::slice( src, 3, 7 );
    }
    SECTION( "Piped syntax" )
    {
        first_n = src | ureact::take( 5 );
        without_first_n = src | ureact::drop( 5 );
        middle = src | ureact::slice( 3, 7 );
    }

    const auto result_first_n = ureact::collect<std::vector>( first_n );
    const auto result_without_first_n = ureact::collect<std::vector>( without_first_n );
    const auto result_middle = ureact::collect<std::vector>( middle );

    // pass integers as events
    for( int i : { 0, 1, 2, 3, 4, 5, -1, 6, 7, 8, 9 } )
        src << i;

    // if we concatenate results of take(N) and drop(N) we receive original set of events
    const std::vector<int> expected_first_n = //
        { 0, 1, 2, 3, 4 };
    const std::vector<int> expected_without_first_n = //
        /*          */ { 5, -1, 6, 7, 8, 9 };
    const std::vector<int> expected_middle = //
        /*    */ { 3, 4, 5, -1 };

    CHECK( result_first_n.get() == expected_first_n );
    CHECK( result_without_first_n.get() == expected_without_first_n );
    CHECK( result_middle.get() == expected_middle );
}

TEST_CASE( "Once" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> first;

    SECTION( "Functional syntax" )
    {
        first = ureact::once( src );
    }
    SECTION( "Piped syntax" )
    {
        first = src | ureact::once;
    }

    const auto result = ureact::collect<std::vector>( first );

    // pass integers as events
    for( int i : { 75, -1, 0 } )
        src << i;

    CHECK( result.get() == std::vector<int>{ 75 } );
}
