//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/cast.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

// Static cast values of event stream
TEST_CASE( "ureact::cast" )
{
    ureact::context ctx;

    auto src = ureact::make_source<short>( ctx );
    ureact::events<int> ints;
    ureact::events<float> floats;

    SECTION( "Functional syntax" )
    {
        ints = ureact::cast<int>( src );
        floats = ureact::cast<float>( src );
    }
    SECTION( "Piped syntax" )
    {
        ints = src | ureact::cast<int>;
        floats = src | ureact::cast<float>;
    }

    const auto ints_values = ureact::collect<std::vector>( ints );
    const auto floats_values = ureact::collect<std::vector>( floats );

    // pass values into src
    for( int i : { -2, -1, 0, 1, 2 } )
        src << static_cast<short>( i );

    // clang-format off
    CHECK( ints_values.get()   == std::vector<int>{   -2,    -1,    0,    1,    2 } );
    CHECK( floats_values.get() == std::vector<float>{ -2.0f, -1.0f, 0.0f, 1.0f, 2.0f } );
    // clang-format on
}
