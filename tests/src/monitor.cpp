//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/monitor.hpp"

#include "doctest_extra.h"
#include "ureact/collect.hpp"
#include "ureact/events.hpp"

// Emits value changes of signal as events
TEST_CASE( "Monitor" )
{
    ureact::context ctx;

    auto src = ureact::make_var<int>( ctx, -1 );
    ureact::events<int> monitored;

    SUBCASE( "Functional syntax" )
    {
        monitored = ureact::monitor( src );
    }
    SUBCASE( "Piped syntax" )
    {
        monitored = src | ureact::monitor;
    }

    const auto result = ureact::collect<std::vector>( monitored );

    // pass values into src
    for( int i : { 0, 0, 0, 1, 1, 2, 3, 0 } )
        src <<= i;

    CHECK( result.get() == std::vector{ 0, 1, 2, 3, 0 } );
}
