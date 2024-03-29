//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/unify.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/count.hpp"

// Transform values of event stream to ureact::unit
TEST_CASE( "ureact::unify" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<> unified;

    SECTION( "Functional syntax" )
    {
        unified = ureact::unify( src );
    }
    SECTION( "Piped syntax" )
    {
        unified = src | ureact::unify;
    }

    const auto events_amount = ureact::count( unified );

    // pass values into src
    for( int i : { -2, -1, 0, 1, 2 } )
        src << i;

    CHECK( events_amount.get() == 5 );
}
