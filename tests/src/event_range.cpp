//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/event_range.hpp"

#include "catch2_extra.hpp"

TEST_CASE( "ureact::event_range<E>" )
{
    SECTION( "empty range" )
    {
        std::vector<int> data;
        ureact::event_range<int> range{ data };

        CHECK( range.empty() );
        CHECK( range.size() == 0 );
        const bool are_begin_end_equal = range.begin() == range.end();
        CHECK( are_begin_end_equal );
        const bool are_rbegin_rend_equal = range.rbegin() == range.rend();
        CHECK( are_rbegin_rend_equal );
    }

    SECTION( "non-empty range" )
    {
        std::vector<int> data{ 0, 1, 2, 3, 4 };
        ureact::event_range<int> range{ data };

        CHECK_FALSE( range.empty() );
        CHECK( range.size() == 5 );
        const bool are_begin_end_equal = range.begin() == range.end();
        CHECK_FALSE( are_begin_end_equal );
        const bool are_rbegin_rend_equal = range.rbegin() == range.rend();
        CHECK_FALSE( are_rbegin_rend_equal );

        const auto direct_range_copy = std::vector<int>{ data.begin(), data.end() };
        const auto reverse_range_copy = std::vector<int>{ data.rbegin(), data.rend() };
        CHECK( direct_range_copy == std::vector<int>{ 0, 1, 2, 3, 4 } );
        CHECK( reverse_range_copy == std::vector<int>{ 4, 3, 2, 1, 0 } );
    }
}
