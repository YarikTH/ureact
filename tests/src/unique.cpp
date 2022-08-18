//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// demonstrate and test special filter function unique
TEST_CASE( "Unique" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> uniq;

    SUBCASE( "Functional syntax" )
    {
        uniq = ureact::unique( src );
    }
    SUBCASE( "Piped syntax" )
    {
        uniq = src | ureact::unique();
    }

    const auto result = ureact::collect<std::vector>( uniq );

    // pass set containing several duplicate elements
    for( int i : { 1, 2, 1, 1, 3, 3, 3, 4, 5, 4 } )
        src << i;

    // expect removing consecutive (adjacent) duplicates
    CHECK( result.get() == std::vector{ 1, 2, 1, 3, 4, 5, 4 } );
}
