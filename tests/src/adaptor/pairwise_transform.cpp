//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/pairwise_transform.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

TEST_CASE( "ureact::pairwise_transform" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> m;
    ureact::events<std::pair<int, int>> p;

    const auto make_pair
        = []( const auto& first, const auto& second ) { return std::make_pair( first, second ); };

    SECTION( "Functional syntax" )
    {
        m = ureact::pairwise_transform( src, std::multiplies<>{} );
        p = ureact::pairwise_transform( src, make_pair );
    }
    SECTION( "Piped syntax" )
    {
        m = src | ureact::pairwise_transform( std::multiplies<>{} );
        p = src | ureact::pairwise_transform( make_pair );
    }

    const auto result_mult = ureact::collect<std::vector>( m );
    const auto result_pair = ureact::collect<std::vector>( p );

    for( int i : { 1, 2, 3, 4, 5, 6 } )
        src << i;

    const std::vector<int> expected_mult = {
        /*         */ 2,
        /*            */ 6,
        /*               */ 12,
        /*                  */ 20,
        /*                     */ 30,
    };

    const std::vector<std::pair<int, int>> expected_pair = {
        /*    */ { 1, 2 },
        /*       */ { 2, 3 },
        /*          */ { 3, 4 },
        /*             */ { 4, 5 },
        /*                */ { 5, 6 },
    };

    CHECK( result_mult.get() == expected_mult );
    CHECK( result_pair.get() == expected_pair );
}
