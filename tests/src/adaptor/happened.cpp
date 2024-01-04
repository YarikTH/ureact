//
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/happened.hpp"

#include "catch2_extra.hpp"
#include "ureact/events.hpp"

// Emits value changes of signal as events
TEST_CASE( "ureact::happened" )
{
    ureact::context ctx;

    auto shit = ureact::make_source<>( ctx );
    ureact::signal<bool> shit_happened;

    SECTION( "Functional syntax" )
    {
        shit_happened = ureact::happened( shit );
    }
    SECTION( "Piped syntax" )
    {
        shit_happened = shit | ureact::happened;
    }

    CHECK_FALSE( shit_happened.get() );

    shit.emit();

    CHECK( shit_happened.get() );
}
