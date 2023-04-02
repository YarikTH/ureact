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

#include "doctest_extra.h"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/transform.hpp"
#include "ureact/events.hpp"

// Based on https://en.cppreference.com/w/cpp/ranges/join_view
TEST_CASE( "JoinStrings" )
{
    ureact::context ctx;

    auto src = ureact::make_source<std::string_view>( ctx );
    ureact::events<char> chars;

    SUBCASE( "Functional syntax" )
    {
        chars = ureact::join( src );
    }
    SUBCASE( "Piped syntax" )
    {
        chars = src | ureact::join;
    }

    const auto result = ureact::collect<std::basic_string>( chars );

    using namespace std::literals;
    for( auto s : { "https:"sv, "//"sv, ""sv, "cppreference"sv, "."sv, "com"sv } )
        src << s;

    CHECK( result.get() == "https://cppreference.com"s );
}

TEST_CASE( "JoinBits" )
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

    SUBCASE( "Functional syntax" )
    {
        bits = ureact::join( bitArrays );
    }
    SUBCASE( "Piped syntax" )
    {
        bits = bitArrays | ureact::join;
    }

    const auto result = ureact::collect<std::vector>( bits );

    src << std::byte{ 0b1010'1011 };

    CHECK( result.get() == std::vector<bool>{ /*0b*/ 1, 0, 1, 0, /*'*/ 1, 0, 1, 1 } );
}

// Based on https://en.cppreference.com/w/cpp/ranges/join_with_view
TEST_CASE( "JoinWith" )
{
    ureact::context ctx;

    auto src = ureact::make_source<std::string_view>( ctx );
    ureact::events<char> chars_1;
    ureact::events<char> chars_2;

    using namespace std::literals;

    SUBCASE( "Functional syntax" )
    {
        chars_1 = ureact::join_with( src, ' ' );
        chars_2 = ureact::join_with( src, ".."sv );
    }
    SUBCASE( "Piped syntax" )
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
