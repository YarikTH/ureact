//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/count.hpp"

#include "doctest_extra.h"
#include "ureact/events.hpp"

// Counts amount of received events into signal<S>
TEST_CASE( "Count" )
{
    ureact::context ctx;

    auto src = ureact::make_source<>( ctx );
    ureact::signal<size_t> counter;
    ureact::signal<int> integer_counter;

    SUBCASE( "Functional syntax" )
    {
        counter = ureact::count( src );               // Default version creates signal<size_t>
        integer_counter = ureact::count_<int>( src ); // Type of signal can be explicitly requested
    }
    SUBCASE( "Piped syntax" )
    {
        counter = src | ureact::count;
        integer_counter = src | ureact::count_<int>;
    }

    src.emit();
    src.emit();

    CHECK( counter.get() == 2 );
    CHECK( integer_counter.get() == 2 );
}
