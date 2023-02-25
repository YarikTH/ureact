//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/stride.hpp"

#include "doctest_extra.h"
#include "ureact/adaptor/collect.hpp"

TEST_CASE( "Stride" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> every_third;

    SUBCASE( "Functional syntax" )
    {
        every_third = ureact::stride( src, 3 );
    }
    SUBCASE( "Piped syntax" )
    {
        every_third = src | ureact::stride( 3 );
    }

    const auto result = ureact::collect<std::vector>( every_third );

    for( int i = 1; i < 13; ++i )
        src << i;

    CHECK( result.get() == std::vector{ 1, 4, 7, 10 } );
}
