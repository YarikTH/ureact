//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/monitor.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

// Emits value changes of signal as events
TEST_CASE( "ureact::monitor" )
{
    ureact::context ctx;

    auto src = ureact::make_var<int>( ctx, -1 );
    ureact::events<int> monitored;

    SECTION( "Functional syntax" )
    {
        monitored = ureact::monitor( src );
    }
    SECTION( "Piped syntax" )
    {
        monitored = src | ureact::monitor;
    }

    const auto result = ureact::collect<std::vector>( monitored );

    // pass values into src
    for( int i : { 0, 0, 0, 1, 1, 2, 3, 0 } )
        src <<= i;

    CHECK( result.get() == std::vector{ 0, 1, 2, 3, 0 } );
}
