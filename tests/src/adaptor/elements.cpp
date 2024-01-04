//
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/elements.hpp"

#include <tuple>
#include <vector>

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

// Based on example on https://en.cppreference.com/w/cpp/ranges/elements_view
TEST_CASE( "ureact::elements" )
{
    ureact::context ctx;

    auto src = ureact::make_source<std::tuple<int, char, std::string>>( ctx );
    ureact::events<int> ints;
    ureact::events<char> chars;
    ureact::events<std::string> strings;

    SECTION( "Functional syntax" )
    {
        ints = ureact::elements<0>( src );
        chars = ureact::elements<1>( src );
        strings = ureact::elements<2>( src );
    }
    SECTION( "Piped syntax" )
    {
        ints = src | ureact::elements<0>;
        chars = src | ureact::elements<1>;
        strings = src | ureact::elements<2>;
    }

    const auto ints_values = ureact::collect<std::vector>( ints );
    const auto chars_values = ureact::collect<std::vector>( chars );
    const auto strings_values = ureact::collect<std::vector>( strings );

    const std::vector<std::tuple<int, char, std::string>> vt{
        { 1, 'A', "α" },
        { 2, 'B', "β" },
        { 3, 'C', "γ" },
        { 4, 'D', "δ" },
        { 5, 'E', "ε" },
    };
    std::copy( vt.begin(), vt.end(), src.begin() );

    // clang-format off
    CHECK( ints_values.get()    == std::vector<int>{          1,   2,   3,   4,   5  } );
    CHECK( chars_values.get()   == std::vector<char>{        'A', 'B', 'C', 'D', 'E' } );
    CHECK( strings_values.get() == std::vector<std::string>{ "α", "β", "γ", "δ", "ε" } );
    // clang-format on
}
