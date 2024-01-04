//
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/values.hpp"

#include <vector>

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

TEST_CASE( "ureact::values" )
{
    ureact::context ctx;

    auto src = ureact::make_source<std::pair<int, std::string>>( ctx );
    ureact::events<std::string> strings;

    SECTION( "Functional syntax" )
    {
        strings = ureact::values( src );
    }
    SECTION( "Piped syntax" )
    {
        strings = src | ureact::values;
    }

    const auto result = ureact::collect<std::vector>( strings );
    const std::vector<std::pair<int, std::string>> vt{
        { 1, "foo" },
        { 2, "bar" },
        { 3, "baz" },
    };
    std::copy( vt.begin(), vt.end(), src.begin() );

    CHECK( result.get() == std::vector<std::string>{ "foo", "bar", "baz" } );
}
