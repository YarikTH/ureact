//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/hold.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/observe.hpp"
#include "ureact/events.hpp"

// hold the last event value as a signal
TEST_CASE( "ureact::hold" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::signal<int> held;

    SECTION( "Functional syntax" )
    {
        held = ureact::hold( src, -1 );
    }
    SECTION( "Piped syntax" )
    {
        held = src | ureact::hold( -1 );
    }

    int changes = 0;
    ureact::observer _ = ureact::observe( held, [&changes]( const auto& ) { ++changes; } );

    // pass values into src
    for( int i : { 0, 0, 1, 1, 2, 3 } )
    {
        src << i;
        CHECK( held.get() == i );
    }

    CHECK( changes == 4 );
}
