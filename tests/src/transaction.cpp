//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "doctest_extra.h"
#include "ureact/count.hpp"
#include "ureact/monitor.hpp"
#include "ureact/ureact.hpp"

// A transaction can be started inside an active transaction
// Only the first transaction takes effect
TEST_CASE( "RecursiveTransactions" )
{
    ureact::context ctx;

    ureact::var_signal src{ ctx, 1 };
    auto change_count = src | ureact::monitor() | ureact::count();

    do_transaction( ctx, [&] {
        src <<= 7;

        do_transaction( ctx, [&] { //
            src <<= 4;
        } );

        src <<= 1;
        src <<= 2;
    } );

    CHECK( change_count.get() == 1 );
}

TEST_CASE( "ComplexTransaction" )
{
    ureact::context ctx;

    ureact::var_signal src{ ctx, 2 };

    auto result = do_transaction(
        ctx,
        [&]( int i ) {      //
            return src + i; // return value from the transaction functor
        },                  //
        3 );                // pass value to the transaction functor

    CHECK( result.is_valid() );
    CHECK( result.get() == 5 );
}
