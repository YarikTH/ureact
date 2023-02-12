//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "doctest_extra.h"
#include "ureact/adaptor/lift.hpp"
#include "ureact/adaptor/observe.hpp"
#include "ureact/transaction.hpp"

TEST_CASE( "Reacting to value changes" )
{
    ureact::context ctx;

    ureact::var_signal<int> x = make_var( ctx, 1 );
    ureact::signal<int> xAbs = ureact::lift( x, []( int value ) { return abs( value ); } );

    std::vector<int> xAbs_values;
    ureact::observe( xAbs, [&]( int new_value ) { xAbs_values.push_back( new_value ); } );

    CHECK( xAbs.get() == 1 );
    CHECK( xAbs_values == std::vector<int>{} );

    x <<= 2;  // xAbs is changed to 2
    x <<= -3; // xAbs is changed to 3
    x <<= 3;  // no output, xAbs is still 3

    do_transaction( ctx, [&] {
        x <<= 4;
        x <<= -2;
        x <<= 3;
    } ); // no output, result value of xAbs is still 3

    CHECK( xAbs_values == std::vector<int>{ 2, 3 } );
}

TEST_CASE( "Changing multiple inputs" )
{
    ureact::context ctx;

    ureact::var_signal<int> a = make_var( ctx, 1 );
    ureact::var_signal<int> b = make_var( ctx, 1 );

    ureact::signal<int> x = a + b;
    ureact::signal<int> y = a + b;
    ureact::signal<int> z = x + y; // 2*(a+b)

    //  a b  //
    //  /X\  //
    // x   y //
    //  \ /  //
    //   z   //

    std::vector<int> z_values;
    ureact::observe( z, [&]( int new_value ) { z_values.push_back( new_value ); } );

    CHECK( z.get() == 4 );
    CHECK( z_values == std::vector<int>{} );

    a <<= 2; // z is changed to 6
    b <<= 2; // z is changed to 8

    do_transaction( ctx, [&] {
        a <<= 4;
        b <<= 4;
    } ); // z is changed to 16

    CHECK( z_values == std::vector<int>{ 6, 8, 16 } );
}
