//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/transaction.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/count.hpp"
#include "ureact/adaptor/lift.hpp"
#include "ureact/adaptor/monitor.hpp"

TEST_CASE( "ureact::transaction" )
{
    ureact::context ctx;

    ureact::var_signal src = ureact::make_var( ctx, 1 );
    auto change_count = ureact::count( ureact::monitor( src ) );

    {
        ureact::transaction _{ ctx };

        src <<= 6;
        src <<= 7;
        src <<= -1;
    }

    CHECK( change_count.get() == 1 );
}

// A transaction can be started inside an active transaction
// Only the first transaction takes effect
TEST_CASE( "Recursive transactions" )
{
    ureact::context ctx;

    ureact::var_signal src = ureact::make_var( ctx, 1 );
    auto change_count = ureact::count( ureact::monitor( src ) );

    {
        ureact::transaction _{ ctx };
        src <<= 7;

        [&] {
            ureact::transaction _{ ctx };
            src <<= 4;
        }();

        src <<= 1;
        src <<= 2;
    }

    CHECK( change_count.get() == 1 );
}

TEST_CASE( "Context death inside transaction" )
{
    auto contextPtr = std::make_unique<ureact::context>();

    {
        ureact::transaction _( *contextPtr );

        contextPtr.reset();
    }

    CHECK( true ); // test should not fail under asan
}
