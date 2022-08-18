//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// zip 3 event sources into 1
// based on example https://en.cppreference.com/w/cpp/ranges/zip_view
TEST_CASE( "Zip" )
{
    ureact::context ctx;

    auto x = ureact::make_source<int>( ctx );
    auto y = ureact::make_source<std::string>( ctx );
    auto z = ureact::make_source<char>( ctx );

    using zipped_t = std::tuple<int, std::string, char>;

    ureact::events<zipped_t> src = zip( x, y, z );

    const auto result = ureact::collect<std::vector>( src );

    // clang-format off
    x <<  1  <<  2  <<  3  <<  4;
    y << "α" << "β" << "γ" << "δ" << "ε";
    z << 'A' << 'B' << 'C' << 'D' << 'E' << 'F';
    // clang-format on

    const std::vector expected = {
        zipped_t{ 1, "α", 'A' },
        zipped_t{ 2, "β", 'B' },
        zipped_t{ 3, "γ", 'C' },
        zipped_t{ 4, "δ", 'D' } //
    };
    CHECK( result.get() == expected );
}
