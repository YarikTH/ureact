//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/drop.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"

// Skips first N elements from the source stream
TEST_CASE( "ureact::drop" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> without_first_n;

    SECTION( "Functional syntax" )
    {
        without_first_n = ureact::drop( src, 5 );
    }
    SECTION( "Piped syntax" )
    {
        without_first_n = src | ureact::drop( 5 );
    }

    const auto result = ureact::collect<std::vector>( without_first_n );

    /*                            N
     *                            v
     *             0  1  2  3  4 [5   6  7  8  9]
     *             v  v  v  v  v  v   v  v  v  v
     */
    for( int i : { 0, 1, 2, 3, 4, 5, -1, 6, 7, 8 } )
        src << i;

    const std::vector<int> expected = //
        /*                   */ { 5, -1, 6, 7, 8 };

    CHECK( result.get() == expected );
}
