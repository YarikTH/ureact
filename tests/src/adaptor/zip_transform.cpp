//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/zip_transform.hpp"

#include "catch2_extra.hpp"
#include "identity.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

// zip 3 event sources into 1
// based on example https://en.cppreference.com/w/cpp/ranges/zip_transform_view
TEST_CASE( "ureact::zip_transform" )
{
    ureact::context ctx;

    auto v1 = ureact::make_source<float>( ctx );
    auto v2 = ureact::make_source<short>( ctx );
    auto v3 = ureact::make_source<int>( ctx );

    UREACT_CLANG_SUPPRESS_WARNING_PUSH
#if UREACT_CLANG >= UREACT_COMPILER( 10, 0, 0 )
    UREACT_CLANG_SUPPRESS_WARNING( "-Wimplicit-int-float-conversion" )
#endif
    const auto add = []( auto a, auto b, auto c ) { return a + b + c; };
    UREACT_CLANG_SUPPRESS_WARNING_POP

    ureact::events<float> src = ureact::zip_transform( add, v1, v2, v3 );

    const auto result = ureact::collect<std::vector>( src );

    // clang-format off
    v1 << 1 << 2 << 3;
    v2 << 1 << 2 << 3 << 4;
    v3 << 1 << 2 << 3 << 4 << 5;
    // clang-format on

    CHECK( result.get() == std::vector<float>{ 3, 6, 9 } );
}

TEST_CASE( "ureact::zip_transform (ref)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );

    ureact::events<int> rvalue = ureact::zip_transform( identity{}, src );
    ureact::events<int> lvalue = ureact::zip_transform( identity_ref{}, src );

    const auto result_rvalue = ureact::collect<std::vector>( rvalue );
    const auto result_lvalue = ureact::collect<std::vector>( lvalue );

    for( int i = 0; i < 5; ++i )
        src << i;

    const std::vector<int> expected = { 0, 1, 2, 3, 4 };
    CHECK( result_rvalue.get() == expected );
    CHECK( result_lvalue.get() == expected );
}
