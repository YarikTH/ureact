//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/snapshot.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/count.hpp"
#include "ureact/adaptor/monitor.hpp"
#include "ureact/adaptor/transform.hpp"

// sets signal value to value of other signal when event is received
TEST_CASE( "ureact::snapshot" )
{
    ureact::context ctx;

    auto trigger = ureact::make_source( ctx );
    auto target = ureact::make_var<int>( ctx, -1 );
    ureact::signal<int> snap;

    SECTION( "Functional syntax" )
    {
        snap = ureact::snapshot( trigger, target );
    }
    SECTION( "Piped syntax" )
    {
        snap = trigger | ureact::snapshot( target );
    }

    SECTION( "Trigger can be any type" )
    {
        auto trigger_int = ureact::transform( trigger, []( ureact::unit ) { return 1; } );
        snap = ureact::snapshot( trigger_int, target );
    }

    const auto changes_count = ureact::count( ureact::monitor( snap ) );

    for( int i = 0; i < 10; ++i )
    {
        // ensure changing the target do not change the out
        target <<= i;
        assert( snap.get() != target.get() );

        // ensure emitting the trigger changes the out
        trigger();
        assert( snap.get() == i );

        // additionally trigger to ensure it does not add additional changes
        for( int j = 0; j < 3; ++j )
            trigger();
    }

    CHECK( changes_count.get() == 10 );
}
