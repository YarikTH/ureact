//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/adjacent.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

TEST_CASE( "ureact::adjacent" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<std::tuple<int, int, int>> a;

    SECTION( "Functional syntax" )
    {
        a = ureact::adjacent<3>( src );
    }
    SECTION( "Piped syntax" )
    {
        a = src | ureact::adjacent<3>;
    }

    const auto result = ureact::collect<std::vector>( a );

    for( int i : { 1, 2, 3, 4, 5, 6 } )
        src << i;

    const std::vector<std::tuple<int, int, int>> expected = {
        /*    */ { 1, 2, 3 },
        /*       */ { 2, 3, 4 },
        /*          */ { 3, 4, 5 },
        /*             */ { 4, 5, 6 },
    };

    CHECK( result.get() == expected );
}
