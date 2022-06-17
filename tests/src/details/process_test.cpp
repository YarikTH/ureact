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

// Emits value of target signal when event is received
TEST_CASE( "Pulse" )
{
    ureact::context ctx;

    auto trigger = ureact::make_event_source( ctx );
    auto target = ureact::make_var<int>( ctx, -1 );
    ureact::events<int> beat;

    // there are two syntax variants and temporary event optimization, so we check them all
    // using subcases
    SUBCASE( "Functional syntax" )
    {
        beat = ureact::pulse( trigger, target );
    }
    SUBCASE( "Functional syntax on temporary" )
    {
        // filter returns temp_events
        // we use them to check filter overloads that receive temp_events rvalue
        // typically we don't need nor std::move nor naming temp events
        ureact::temp_events temp = ureact::filter( trigger, always_true );
        beat = ureact::pulse( std::move( temp ), target );
        CHECK_FALSE( temp.was_op_stolen() ); // there is no temp_events optimization here
    }

    SUBCASE( "Piped syntax" )
    {
        beat = trigger | ureact::pulse( target );
    }
    SUBCASE( "Piped syntax on temporary" )
    {
        ureact::temp_events temp = trigger | ureact::filter( always_true );
        beat = std::move( temp ) | ureact::pulse( target );
        CHECK_FALSE( temp.was_op_stolen() ); // there is no temp_events optimization here
    }

    SUBCASE( "Trigger can be any type" )
    {
        beat = ureact::transform( trigger, []( ureact::token ) { return 1; } )
             | ureact::pulse( target );
    }

    auto result = make_collector( beat );

    target <<= 1;
    for( int i = 0; i < 2; ++i )
        trigger();

    ctx.do_transaction( [&]() {
        target <<= 6;
        for( int i = 0; i < 3; ++i )
            trigger();
    } );

    // we expect first value twice and second value thrice
    const std::vector<int> expected = { 1, 1, 6, 6, 6 };
    CHECK( result.get() == expected );
}