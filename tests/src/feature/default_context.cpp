//
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/lift.hpp"
#include "ureact/adaptor/merge.hpp"
#include "ureact/adaptor/monitor.hpp"
#include "ureact/context.hpp"
#include "ureact/events.hpp"
#include "ureact/signal.hpp"
#include "ureact/transaction.hpp"

TEST_CASE( "ureact::default_context (signal<S>)" )
{
    using namespace ureact::default_context;

    ureact::var_signal src1 = make_var( 1 );
    ureact::signal src2 = make_const( 2 );
    ureact::signal result = src1 + src2;

    CHECK( result.get() == 3 );
    src1 <<= 2;
    CHECK( result.get() == 4 );
}

TEST_CASE( "ureact::default_context (events<E>)" )
{
    using namespace ureact::default_context;

    ureact::event_source src = make_source<int>();
    ureact::events nev = make_never<int>();
    ureact::events events = ureact::merge( src, nev );
    ureact::signal result = ureact::collect<std::vector>( events );

    src << 1 << 2 << 3;
    CHECK( result.get() == std::vector{ 1, 2, 3 } );
}

TEST_CASE( "ureact::default_context (transactions)" )
{
    using namespace ureact::default_context;

    ureact::var_signal src = make_var( 1 );
    ureact::signal src_values = ureact::monitor( src ) | ureact::collect<std::vector>;

    {
        default_transaction _;
        src <<= 2;
        src <<= 1;
        src <<= 4;
    }

    CHECK( src_values.get() == std::vector{ 4 } );
}
