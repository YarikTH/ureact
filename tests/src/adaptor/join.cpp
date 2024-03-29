//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/join.hpp"

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/transform.hpp"
#include "ureact/events.hpp"

// Based on https://en.cppreference.com/w/cpp/ranges/join_view
TEST_CASE( "ureact::join (strings)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<std::string_view>( ctx );
    ureact::events<char> chars;

    SECTION( "Functional syntax" )
    {
        chars = ureact::join( src );
    }
    SECTION( "Piped syntax" )
    {
        chars = src | ureact::join;
    }

    const auto result = ureact::collect<std::basic_string>( chars );

    using namespace std::literals;
    for( auto s : { "https:"sv, "//"sv, ""sv, "cppreference"sv, "."sv, "com"sv } )
        src << s;

    CHECK( result.get() == "https://cppreference.com"s );
}

TEST_CASE( "ureact::join (bits)" )
{
    ureact::context ctx;

    constexpr auto byte2bits = []( const std::byte b ) {
        return std::array{
            bool( b & std::byte( 1u << 7 ) ),
            bool( b & std::byte( 1u << 6 ) ),
            bool( b & std::byte( 1u << 5 ) ),
            bool( b & std::byte( 1u << 4 ) ),
            bool( b & std::byte( 1u << 3 ) ),
            bool( b & std::byte( 1u << 2 ) ),
            bool( b & std::byte( 1u << 1 ) ),
            bool( b & std::byte( 1u << 0 ) ),
        };
    };

    auto src = ureact::make_source<std::byte>( ctx );
    auto bitArrays = ureact::transform( src, byte2bits );
    ureact::events<bool> bits;

    SECTION( "Functional syntax" )
    {
        bits = ureact::join( bitArrays );
    }
    SECTION( "Piped syntax" )
    {
        bits = bitArrays | ureact::join;
    }

    const auto result = ureact::collect<std::vector>( bits );

    src << std::byte{ 0b1010'1011 };

    CHECK( result.get() == std::vector<bool>{ /*0b*/ 1, 0, 1, 0, /*'*/ 1, 0, 1, 1 } );
}
