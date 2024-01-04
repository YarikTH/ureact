//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/monitor_change.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

// Emits value changes of signal as events
TEST_CASE( "ureact::monitor_change" )
{
    ureact::context ctx;

    auto src = ureact::make_var<int>( ctx, -1 );
    ureact::events<std::pair<int, int>> monitored;

    SECTION( "Functional syntax" )
    {
        monitored = ureact::monitor_change( src );
    }
    SECTION( "Piped syntax" )
    {
        monitored = src | ureact::monitor_change;
    }

    const auto result = ureact::collect<std::vector>( monitored );

    // pass values into src
    for( int i : { 0, 0, 0, 1, 1 } )
        src <<= i;

    CHECK( result.get() == std::vector{ std::pair{ -1, 0 }, std::pair{ 0, 1 } } );
}
