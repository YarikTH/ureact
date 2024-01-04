//
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/pairwise_filter.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

TEST_CASE( "ureact::pairwise_filter" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> u;

    const auto uniq = []( const auto& prev, const auto& current ) { return !( current == prev ); };

    SECTION( "Functional syntax" )
    {
        u = ureact::pairwise_filter( src, uniq );
    }
    SECTION( "Piped syntax" )
    {
        u = src | ureact::pairwise_filter( uniq );
    }

    const auto result = ureact::collect<std::vector>( u );

    for( int i : { 1, 1, 3, 3, 5, 6 } )
        src << i;

    CHECK( result.get() == std::vector{ 1, 3, 5, 6 } );
}
