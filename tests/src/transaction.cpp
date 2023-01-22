//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/transaction.hpp"

#include "doctest_extra.h"
#include "ureact/count.hpp"
#include "ureact/lift.hpp"
#include "ureact/monitor.hpp"

TEST_CASE( "TransactionGuard" )
{
    ureact::context ctx;

    ureact::var_signal src = ureact::make_var( ctx, 1 );
    auto change_count = src | ureact::monitor() | ureact::count();

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
TEST_CASE( "RecursiveTransactions" )
{
    ureact::context ctx;

    ureact::var_signal src = ureact::make_var( ctx, 1 );
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

    ureact::var_signal src = ureact::make_var( ctx, 2 );
    ureact::signal<int> result;

    SUBCASE( "lambda" )
    {
        result = do_transaction(
            ctx,
            [&]( int i ) {      //
                return src + i; // return value from the transaction functor
            },                  //
            3 );                // pass value to the transaction functor
    }
    SUBCASE( "member" )
    {
        struct Foo
        {
            int i;
            const ureact::signal<int>& src;

            [[nodiscard]] ureact::signal<int> add_to_signal() const
            {
                return src + i;
            }
        };

        Foo foo{ 3, src };
        result = do_transaction( ctx, &Foo::add_to_signal, foo );
    }

    CHECK( result.is_valid() );
    CHECK( result.get() == 5 );
}
