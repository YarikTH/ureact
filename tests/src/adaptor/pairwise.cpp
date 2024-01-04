//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/pairwise.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

TEST_CASE( "ureact::pairwise" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<std::tuple<int, int>> a;

    SECTION( "Functional syntax" )
    {
        a = ureact::pairwise( src );
    }
    SECTION( "Piped syntax" )
    {
        a = src | ureact::pairwise;
    }

    const auto result = ureact::collect<std::vector>( a );

    for( int i : { 1, 2, 3, 4, 5, 6 } )
        src << i;

    const std::vector<std::tuple<int, int>> expected = {
        /*    */ { 1, 2 },
        /*       */ { 2, 3 },
        /*          */ { 3, 4 },
        /*             */ { 4, 5 },
        /*                */ { 5, 6 },
    };

    CHECK( result.get() == expected );
}
