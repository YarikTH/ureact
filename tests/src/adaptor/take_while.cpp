//
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/take_while.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/lift.hpp"

// Keeps the first elements of the source stream that satisfy the predicate
TEST_CASE( "ureact::take_while" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> before_negative;

    const auto is_not_negative = [&]( auto i ) { return i >= 0; };

    SECTION( "Functional syntax" )
    {
        before_negative = ureact::take_while( src, is_not_negative );
    }
    SECTION( "Piped syntax" )
    {
        before_negative = src | ureact::take_while( is_not_negative );
    }

    const auto result = ureact::collect<std::vector>( before_negative );

    /*                               -1
     *            [                ]  v)
     */
    for( int i : { 0, 1, 2, 3, 4, 5, -1, 6, 7, 8 } )
        src << i;

    const std::vector<int> expected = //
        /*    */ { 0, 1, 2, 3, 4, 5 };
    CHECK( result.get() == expected );
}

// Keeps the first elements of the source stream that satisfy the predicate
// that depends on signal values
TEST_CASE( "ureact::take_while (synced)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> before_overflow;

    const ureact::var_signal<int> blackjack = make_var( ctx, 21 );
    const ureact::signal<int> sum = ureact::fold( src, 0, std::plus<>() );
    const ureact::signal<bool> overflown = sum >= blackjack;

    const auto is_not_overflown
        = []( int /*e*/, int sum_value, int blackjack ) { return sum_value < blackjack; };
    const auto is_not_overflown_2 = []( int /*e*/, bool overflown ) { return !overflown; };

    SECTION( "Functional syntax" )
    {
        before_overflow = ureact::take_while( src, with( sum, blackjack ), is_not_overflown );
    }
    SECTION( "Piped syntax" )
    {
        before_overflow = src | ureact::take_while( with( sum, blackjack ), is_not_overflown );
    }
    SECTION( "Calculated bool condition" )
    {
        before_overflow = ureact::take_while( src, with( overflown ), is_not_overflown_2 );
    }

    const auto result = ureact::collect<std::vector>( before_overflow );

    /*                         21
     *                         v
     *       sum: [10  15 16 19]25 29 30 41
     *             v   v  v  v  v  v  v  v
     */
    for( int i : { 10, 5, 1, 3, 6, 4, 1, 11 } )
        src << i;

    const std::vector<int> expected = //
        /*    */ { 10, 5, 1, 3 };
    CHECK( result.get() == expected );
}
