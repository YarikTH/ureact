//
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/slice.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"

// Keeps given range (begin through end-1) of elements from the source stream
TEST_CASE( "ureact::slice" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> middle;

    SECTION( "Functional syntax" )
    {
        middle = ureact::slice( src, 3, 7 );
    }
    SECTION( "Piped syntax" )
    {
        middle = src | ureact::slice( 3, 7 );
    }

    const auto result = ureact::collect<std::vector>( middle );

    /*                    start         end
     *                      v            v
     *             0  1  2 [3  4  5   6] 7) 8  9
     *             v  v  v  v  v  v   v  v  v  v
     */
    for( int i : { 0, 1, 2, 3, 4, 5, -1, 6, 7, 8 } )
        src << i;

    const std::vector<int> expected = //
        /*             */ { 3, 4, 5, -1 };

    CHECK( result.get() == expected );
}
