//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// Collects received events into signal<ContT<E>>
TEST_CASE( "Collect" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::signal<std::vector<int>> collected_vec;
    ureact::signal<std::deque<int>> collected_deq;
    ureact::signal<std::list<int>> collected_lst;
    ureact::signal<std::set<int>> collected_set;

    SUBCASE( "Functional syntax" )
    {
        collected_vec = ureact::collect<std::vector>( src );
        collected_deq = ureact::collect<std::deque>( src );
        collected_lst = ureact::collect<std::list>( src );
        collected_set = ureact::collect<std::set>( src );
    }
    SUBCASE( "Piped syntax" )
    {
        collected_vec = src | ureact::collect<std::vector>();
        collected_deq = src | ureact::collect<std::deque>();
        collected_lst = src | ureact::collect<std::list>();
        collected_set = src | ureact::collect<std::set>();
    }

    for( int i : { 1, 2, 3 } )
        src << i;

    CHECK_EQ( collected_vec.get(), std::vector{ 1, 2, 3 } );
    CHECK_EQ( collected_deq.get(), std::deque{ 1, 2, 3 } );
    CHECK_EQ( collected_lst.get(), std::list{ 1, 2, 3 } );
    CHECK_EQ( collected_set.get(), std::set{ 1, 2, 3 } );
}
