//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/merge.hpp"

#include "doctest_extra.h"
#include "ureact/collect.hpp"

// merge 3 event sources into 1
TEST_CASE( "Merge" )
{
    ureact::context ctx;

    auto src1 = ureact::make_source<int>( ctx );
    auto src2 = ureact::make_source<int>( ctx );
    auto src3 = ureact::make_source<int>( ctx );

    ureact::events<int> src = merge( src1, src2, src3 );

    const auto result = ureact::collect<std::vector>( src );

    src1 << 1 << 5;
    src2 << -1;
    src3 << 9;
    src2 << 0;

    const std::vector<int> expected = { 1, 5, -1, 9, 0 };
    CHECK( result.get() == expected );
}
