//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/count.hpp"

#include "catch2_extra.hpp"
#include "ureact/events.hpp"

// Counts amount of received events into signal<S>
TEST_CASE( "ureact::count" )
{
    ureact::context ctx;

    auto src = ureact::make_source<>( ctx );
    ureact::signal<size_t> counter;

    SECTION( "Functional syntax" )
    {
        counter = ureact::count( src ); // Default version creates signal<size_t>
    }
    SECTION( "Piped syntax" )
    {
        counter = src | ureact::count;
    }

    src.emit();
    src.emit();

    CHECK( counter.get() == 2 );
}

// Counts amount of received events into signal<S> of requested S type
TEST_CASE( "ureact::count_as" )
{
    ureact::context ctx;

    auto src = ureact::make_source<>( ctx );
    ureact::signal<int> integer_counter;

    SECTION( "Functional syntax" )
    {
        integer_counter
            = ureact::count_as<int>( src ); // Type of signal can be explicitly requested
    }
    SECTION( "Piped syntax" )
    {
        integer_counter = src | ureact::count_as<int>;
    }

    src.emit();
    src.emit();

    CHECK( integer_counter.get() == 2 );
}
