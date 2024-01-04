//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/stride.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"

TEST_CASE( "ureact::stride" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> every_third;

    SECTION( "Functional syntax" )
    {
        every_third = ureact::stride( src, 3 );
    }
    SECTION( "Piped syntax" )
    {
        every_third = src | ureact::stride( 3 );
    }

    const auto result = ureact::collect<std::vector>( every_third );

    for( int i = 1; i < 13; ++i )
        src << i;

    CHECK( result.get() == std::vector{ 1, 4, 7, 10 } );
}
