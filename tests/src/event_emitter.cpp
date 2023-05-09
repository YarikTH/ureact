//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/event_emitter.hpp"

#include <algorithm>

#include "catch2_extra.hpp"

TEST_CASE( "EventEmitter" )
{
    std::vector<int> data;
    ureact::event_emitter<int> emitter{ data };
    auto _2 = 2;

    SECTION( "operator =" )
    {
        emitter = 1;  // R-value
        emitter = _2; // L-value
    }
    SECTION( "stream" )
    {
        emitter << 1   // R-value
                << _2; // L-value
    }
    SECTION( "stl iterator" )
    {
        std::generate_n( emitter, 1, [] { return 1; } );                   // R-value
        std::generate_n( emitter, 1, [&]() -> const int& { return _2; } ); // L-value
    }

    CHECK( data == std::vector<int>{ 1, 2 } );
}
