//
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/changed_to.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/count.hpp"

// Emits unit when target signal was changed to value
TEST_CASE( "ureact::changed_to" )
{
    ureact::context ctx;

    auto src = ureact::make_var<int>( ctx, -1 );
    ureact::events<> changes_to_zero;

    SECTION( "Functional syntax" )
    {
        changes_to_zero = ureact::changed_to( src, 0 );
    }
    SECTION( "Piped syntax" )
    {
        changes_to_zero = src | ureact::changed_to( 0 );
    }

    const auto changes_to_zero_count = ureact::count( changes_to_zero );

    // pass values into src
    for( int i : { 0, 0, 0, 1, 1, 2, 3, 0 } )
        src <<= i;

    // only fact of changes matters [0, 1, 2, 3, 0]
    CHECK( changes_to_zero_count.get() == 2 );
}
