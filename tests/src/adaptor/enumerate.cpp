//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/enumerate.hpp"

#include <tuple>
#include <vector>

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

// Based on example on https://en.cppreference.com/w/cpp/ranges/enumerate_view
TEST_CASE( "ureact::enumerate" )
{
    ureact::context ctx;

    auto src = ureact::make_source<char>( ctx );
    ureact::events<std::tuple<size_t, char>> enumerated;

    SECTION( "Functional syntax" )
    {
        enumerated = ureact::enumerate( src );
    }
    SECTION( "Piped syntax" )
    {
        enumerated = src | ureact::enumerate;
    }

    const auto result = ureact::collect<std::vector>( enumerated );

    for( char c : { 'A', 'B', 'C', 'D' } )
        src << c;

    CHECK( result.get()
           == std::vector<std::tuple<size_t, char>>{
               { 0, 'A' },
               { 1, 'B' },
               { 2, 'C' },
               { 3, 'D' },
           } );
}
