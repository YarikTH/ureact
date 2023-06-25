//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/drop_while.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/lift.hpp"

// Skips the first elements of the source stream that satisfy the predicate
TEST_CASE( "DropWhile" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> from_negative;

    const auto is_not_negative = [&]( auto i ) { return i >= 0; };

    SECTION( "Functional syntax" )
    {
        from_negative = ureact::drop_while( src, is_not_negative );
    }
    SECTION( "Piped syntax" )
    {
        from_negative = src | ureact::drop_while( is_not_negative );
    }

    const auto result = ureact::collect<std::vector>( from_negative );

    /*                               -1
     *                              [ v         ]
     */
    for( int i : { 0, 1, 2, 3, 4, 5, -1, 6, 7, 8 } )
        src << i;

    const std::vector<int> expected = //
        /*                      */ { -1, 6, 7, 8 };
    CHECK( result.get() == expected );
}

// Skips the first elements of the source stream that satisfy the predicate
// that depends on signal values
TEST_CASE( "DropWhileSynced" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> from_overflow;

    const ureact::var_signal<int> blackjack = make_var( ctx, 21 );
    const ureact::signal<int> sum = ureact::fold( src, 0, std::plus<>() );
    const ureact::signal<bool> overflown = sum >= blackjack;

    const auto is_not_overflown
        = []( int /*e*/, int sum_value, int blackjack ) { return sum_value < blackjack; };
    const auto is_not_overflown_2 = []( int /*e*/, bool overflowed ) { return !overflowed; };

    SECTION( "Functional syntax" )
    {
        from_overflow = ureact::drop_while( src, with( sum, blackjack ), is_not_overflown );
    }
    SECTION( "Piped syntax" )
    {
        from_overflow = src | ureact::drop_while( with( sum, blackjack ), is_not_overflown );
    }
    SECTION( "Calculated bool condition" )
    {
        from_overflow = ureact::drop_while( src, with( overflown ), is_not_overflown_2 );
    }

    const auto result = ureact::collect<std::vector>( from_overflow );

    /*                         21
     *                         v
     *        sum: 10  15 16 19[25 29 30 41]
     *             v   v  v  v  v  v  v  v
     */
    for( int i : { 10, 5, 1, 3, 6, 4, 1, 11 } )
        src << i;

    const std::vector<int> expected = //
        /*                 */ { 6, 4, 1, 11 };
    CHECK( result.get() == expected );
}
