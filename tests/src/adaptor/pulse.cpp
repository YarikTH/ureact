//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/pulse.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/transform.hpp"
#include "ureact/transaction.hpp"

// Emits value of target signal when event is received
TEST_CASE( "ureact::pulse" )
{
    ureact::context ctx;

    auto trigger = ureact::make_source( ctx );
    auto target = ureact::make_var<int>( ctx, -1 );
    ureact::events<int> beat;

    SECTION( "Functional syntax" )
    {
        beat = ureact::pulse( trigger, target );
    }
    SECTION( "Piped syntax" )
    {
        beat = trigger | ureact::pulse( target );
    }

    SECTION( "Trigger can be any type" )
    {
        auto trigger_int = ureact::transform( trigger, []( ureact::unit ) { return 1; } );
        beat = ureact::pulse( trigger_int, target );
    }

    auto result = ureact::collect<std::vector>( beat );

    target <<= 1;
    for( int i = 0; i < 2; ++i )
        trigger();

    {
        ureact::transaction _{ ctx };
        target <<= 6;
        for( int i = 0; i < 3; ++i )
            trigger();
    }

    // we expect first value twice and second value thrice
    CHECK( result.get() == std::vector{ 1, 1, 6, 6, 6 } );
}
