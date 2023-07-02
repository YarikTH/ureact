//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/keys.hpp"

#include <string>
#include <utility>
#include <vector>

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

TEST_CASE( "ureact::keys" )
{
    ureact::context ctx;

    auto src = ureact::make_source<std::pair<int, std::string>>( ctx );
    ureact::events<int> ints;

    SECTION( "Functional syntax" )
    {
        ints = ureact::keys( src );
    }
    SECTION( "Piped syntax" )
    {
        ints = src | ureact::keys;
    }

    const auto result = ureact::collect<std::vector>( ints );
    const std::vector<std::pair<int, std::string>> vt{
        { 1, "foo" },
        { 2, "bar" },
        { 3, "baz" },
    };
    std::copy( vt.begin(), vt.end(), src.begin() );

    CHECK( result.get() == std::vector<int>{ 1, 2, 3 } );
}
