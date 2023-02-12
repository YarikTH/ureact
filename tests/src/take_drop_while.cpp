//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/take_drop_while.hpp"

#include "doctest_extra.h"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/lift.hpp"

// filters that take first elements or skip first elements according to given predicate
TEST_CASE( "TakeOrDropWhile" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> before_negative;
    ureact::events<int> from_negative;

    const auto is_not_negative = [&]( auto i ) { return i >= 0; };

    SUBCASE( "Functional syntax" )
    {
        before_negative = ureact::take_while( src, is_not_negative );
        from_negative = ureact::drop_while( src, is_not_negative );
    }
    SUBCASE( "Piped syntax" )
    {
        before_negative = src | ureact::take_while( is_not_negative );
        from_negative = src | ureact::drop_while( is_not_negative );
    }

    const auto result_before_negative = ureact::collect<std::vector>( before_negative );
    const auto result_from_negative = ureact::collect<std::vector>( from_negative );

    // pass integers as events
    for( int i : { 0, 1, 2, 3, 4, 5, -1, 6, 7, 8, 9 } )
        src << i;

    // if we concatenate results of take_while(pred) and drop_while(pred) we receive original vector
    const std::vector<int> expected_before_negative = //
        { 0, 1, 2, 3, 4, 5 };
    const std::vector<int> expected_from_negative = //
        /*             */ { -1, 6, 7, 8, 9 };
    CHECK( result_before_negative.get() == expected_before_negative );
    CHECK( result_from_negative.get() == expected_from_negative );
}

// filters that take first elements or skip first elements according to given predicate
// that depends on signal values
TEST_CASE( "TakeOrDropWhileSynced" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> before_overflow;
    ureact::events<int> from_overflow;

    const ureact::var_signal<int> blackjack = make_var( ctx, 21 );
    const ureact::signal<int> sum = ureact::fold( src, 0, std::plus<>() );
    const ureact::signal<bool> overflowed = sum >= blackjack;

    const auto is_not_overflowed
        = []( int /*e*/, int sum_value, int blackjack ) { return sum_value < blackjack; };
    const auto is_not_overflowed_2 = []( int /*e*/, bool overflowed ) { return !overflowed; };

    SUBCASE( "Functional syntax" )
    {
        before_overflow = ureact::take_while( src, with( sum, blackjack ), is_not_overflowed );
        from_overflow = ureact::drop_while( src, with( sum, blackjack ), is_not_overflowed );
    }
    SUBCASE( "Piped syntax" )
    {
        before_overflow = src | ureact::take_while( with( sum, blackjack ), is_not_overflowed );
        from_overflow = src | ureact::drop_while( with( sum, blackjack ), is_not_overflowed );
    }
    SUBCASE( "Calculated bool condition" )
    {
        before_overflow = ureact::take_while( src, with( overflowed ), is_not_overflowed_2 );
        from_overflow = ureact::drop_while( src, with( overflowed ), is_not_overflowed_2 );
    }

    const auto result_before_overflow = ureact::collect<std::vector>( before_overflow );
    const auto result_from_overflow = ureact::collect<std::vector>( from_overflow );

    // pass integers as events
    for( int i : { 10, 5, 1, 3 /*19*/, 6 /*25*/, 4, 1, 11 } )
        src << i;

    // if we concatenate results of take_while(pred) and drop_while(pred) we receive original vector
    const std::vector<int> expected_before_overflow = //
        { 10, 5, 1, 3 };
    const std::vector<int> expected_from_overflow = //
        /*       */ { 6, 4, 1, 11 };
    CHECK( result_before_overflow.get() == expected_before_overflow );
    CHECK( result_from_overflow.get() == expected_from_overflow );
}
