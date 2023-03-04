//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/merge.hpp"

// clang-6 is supported by ureact, but it doesn't have working version of std::variant
#if !defined( __clang__ ) || __clang_major__ >= 7
#    define HAS_WORKING_VARIANT true
#else
#    define HAS_WORKING_VARIANT false
#endif

#if HAS_WORKING_VARIANT
#    include <variant>
#endif

#include "doctest_extra.h"
#include "ureact/adaptor/collect.hpp"
#include "ureact/events.hpp"

// merge 3 event sources into 1
TEST_CASE( "Merge" )
{
    ureact::context ctx;

    auto src1 = ureact::make_source<int>( ctx );
    auto src2 = ureact::make_source<int>( ctx );
    auto src3 = ureact::make_source<int>( ctx );

    ureact::events<int> src = ureact::merge( src1, src2, src3 );

    const auto result = ureact::collect<std::vector>( src );

    src1 << 1 << 5;
    src2 << -1;
    src3 << 9;
    src2 << 0;

    const std::vector<int> expected = { 1, 5, -1, 9, 0 };
    CHECK( result.get() == expected );
}

TEST_CASE( "MergeSeveralTypes" )
{
    ureact::context ctx;

    auto src1 = ureact::make_source<int>( ctx );
    auto src2 = ureact::make_source<int64_t>( ctx );
    auto src3 = ureact::make_source<bool>( ctx );
    auto src4 = ureact::make_source<unsigned>( ctx );

    ureact::events<int64_t> src = ureact::merge( src1, src2, src3, src4 );

    const auto result = ureact::collect<std::vector>( src );

    src1 << 1 << 5;
    src2 << INT64_MIN;
    src3 << true;
    src2 << INT64_MAX;
    src4 << 42;

    const std::vector<int64_t> expected = { 1, 5, INT64_MIN, 1, INT64_MAX, 42 };
    CHECK( result.get() == expected );
}

#if HAS_WORKING_VARIANT
TEST_CASE( "MergeAs" )
{
    ureact::context ctx;

    auto src1 = ureact::make_source<int>( ctx );
    auto src2 = ureact::make_source<std::string>( ctx );

    using merged_type = std::variant<std::string, int>;

    ureact::events src = ureact::merge_as<merged_type>( src1, src2 );

    const auto result = ureact::collect<std::vector>( src );

    src1 << 1 << 5;
    src2 << "wtf?";

    const std::vector<merged_type> expected = { 1, 5, "wtf?" };
    CHECK( result.get() == expected );
}
#endif
