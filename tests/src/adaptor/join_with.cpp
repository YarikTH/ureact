//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/join_with.hpp"

#include <string>
#include <string_view>

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

// Based on https://en.cppreference.com/w/cpp/ranges/join_with_view
TEST_CASE( "ureact::join_with" )
{
    ureact::context ctx;

    auto src = ureact::make_source<std::string_view>( ctx );
    ureact::events<char> chars_1;
    ureact::events<char> chars_2;

    using namespace std::literals;

    SECTION( "Functional syntax" )
    {
        chars_1 = ureact::join_with( src, ' ' );
        chars_2 = ureact::join_with( src, ".."sv );
    }
    SECTION( "Piped syntax" )
    {
        chars_1 = src | ureact::join_with( ' ' );
        chars_2 = src | ureact::join_with( ".."sv );
    }

    const auto result_1 = ureact::collect<std::basic_string>( chars_1 );
    const auto result_2 = ureact::collect<std::basic_string>( chars_2 );

    using namespace std::literals;
    for( auto s : { "This"sv, "is"sv, "a"sv, "test."sv } )
        src << s;

    CHECK( result_1.get() == "This is a test."s );
    CHECK( result_2.get() == "This..is..a..test."s );
}
