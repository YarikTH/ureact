//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

// Emits value changes of signal as events
TEST_CASE( "Monitor" )
{
    ureact::context ctx;

    auto src = ureact::make_var<int>( ctx, -1 );
    ureact::events<int> monitored;
    ureact::events<> changes;
    ureact::events<> changes_to_zero;

    // there are two syntax variants, so we check them all
    // using subcases
    SUBCASE( "Functional syntax" )
    {
        monitored = ureact::monitor( src );
        changes = ureact::changed( src );
        changes_to_zero = ureact::changed_to( src, 0 );
    }
    //    SUBCASE( "Piped syntax" )
    //    {
    //        // TODO: the is a problem with ambigues operator|
    //        // maybe I need to create lambda wrapper called algorithm or something
    //        // to disable signal<S> | func(S) overload
    //        monitored = src | ureact::monitor();
    //        changes = src | ureact::changed();
    //        changes_to_zero = src | ureact::changed_to( 0 );
    //    }

    const auto result = make_collector( monitored );
    const auto changes_count = make_counter( changes );
    const auto changes_to_zero_count = make_counter( changes_to_zero );

    // pass values into src
    for( int i : { 0, 0, 0, 1, 1, 2, 3, 0 } )
        src <<= i;

    const std::vector<int> expected = { 0, 1, 2, 3, 0 };
    CHECK( result.get() == expected );
    CHECK( changes_count.get() == 5 );
    CHECK( changes_to_zero_count.get() == 2 );
}
