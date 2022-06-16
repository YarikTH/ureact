//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

namespace
{
//
} // namespace

// hold the last event value as a signal
TEST_CASE( "Hold" )
{
    ureact::context ctx;

    auto src = ureact::make_event_source<int>( ctx );
    ureact::signal<int> held;

    // there are two syntax variants and temporary event optimization, so we check them all
    // using subcases
    SUBCASE( "Functional syntax" )
    {
        held = ureact::hold( src, -1 );
    }
    SUBCASE( "Functional syntax on temporary" )
    {
        // filter returns temp_events
        // we use them to check filter overloads that receive temp_events rvalue
        // typically we don't need nor std::move nor naming temp events
        ureact::temp_events temp = ureact::filter( src, always_true );
        held = ureact::hold( std::move( temp ), -1 );
        CHECK_FALSE( temp.was_op_stolen() ); // there is no temp_events optimization here
    }

    SUBCASE( "Piped syntax" )
    {
        held = src | ureact::hold( -1 );
    }
    SUBCASE( "Piped syntax on temporary" )
    {
        ureact::temp_events temp = src | ureact::filter( always_true );
        held = std::move( temp ) | ureact::hold( -1 );
        CHECK_FALSE( temp.was_op_stolen() ); // there is no temp_events optimization here
    }

    int changes = 0;
    observe( held, [&changes]( const auto& ) { ++changes; } );

    // pass 0-9 values into src
    for( int i : { 0, 0, 1, 1, 2, 3 } )
    {
        src << i;
        CHECK( held.get() == i );
    }

    CHECK( changes == 4 );
}
