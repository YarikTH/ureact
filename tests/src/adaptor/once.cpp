//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/once.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"

TEST_CASE( "Once" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> first;

    SECTION( "Functional syntax" )
    {
        first = ureact::once( src );
    }
    SECTION( "Piped syntax" )
    {
        first = src | ureact::once;
    }

    const auto result = ureact::collect<std::vector>( first );

    for( int i : { 75, -1, 0 } )
        src << i;

    CHECK( result.get() == std::vector<int>{ 75 } );
}
