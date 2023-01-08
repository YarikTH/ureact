//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/snapshot.hpp"

#include "doctest_extra.h"
#include "ureact/count.hpp"
#include "ureact/monitor.hpp"
#include "ureact/transform.hpp"

// sets signal value to value of other signal when event is received
TEST_CASE( "Snapshot" )
{
    ureact::context ctx;

    auto trigger = ureact::make_source( ctx );
    auto target = ureact::make_var<int>( ctx, -1 );
    ureact::signal<int> snap;

    SUBCASE( "Functional syntax" )
    {
        snap = ureact::snapshot( trigger, target );
    }
    SUBCASE( "Piped syntax" )
    {
        snap = trigger | ureact::snapshot( target );
    }

    SUBCASE( "Trigger can be any type" )
    {
        snap = ureact::transform( trigger, []( ureact::unit ) { return 1; } )
             | ureact::snapshot( target );
    }

    const auto changes_count = monitor( snap ) | ureact::count();

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
