//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/changed.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/count.hpp"

// Emits unit when target signal was changed
TEST_CASE( "ureact::changed" )
{
    ureact::context ctx;

    auto src = ureact::make_var<int>( ctx, -1 );
    ureact::events<> changes;

    SECTION( "Functional syntax" )
    {
        changes = ureact::changed( src );
    }
    SECTION( "Piped syntax" )
    {
        changes = src | ureact::changed;
    }

    const auto changes_count = ureact::count( changes );

    // pass values into src
    for( int i : { 0, 0, 0, 1, 1, 2, 3, 0 } )
        src <<= i;

    // only fact of changes matters [0, 1, 2, 3, 0]
    CHECK( changes_count.get() == 5 );
}
