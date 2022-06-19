//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

// filter only even integer events
// our check function depends only on event value
TEST_CASE( "Filter" )
{
    ureact::context ctx;

    auto src = ureact::make_event_source<int>( ctx );
    ureact::events<int> filtered;
    const auto is_even = []( auto i ) { return i % 2 == 0; };

    // there are two syntax variants, so we check them all using subcases
    SUBCASE( "Functional syntax" )
    {
        filtered = ureact::filter( src, is_even );
    }
    SUBCASE( "Piped syntax" )
    {
        filtered = src | ureact::filter( is_even );
    }

    const auto result = make_collector( filtered );

    // pass 0-9 values into src
    for( int i = 0; i < 10; ++i )
    {
        src << i;
    }

    // we expect only even numbers passed our filter
    const std::vector<int> expected = { 0, 2, 4, 6, 8 };
    CHECK( result.get() == expected );
}

// filter integer events which values are in range which is presented in the form
// of signals
TEST_CASE( "FilterSynced" )
{
    ureact::context ctx;

    auto src = ureact::make_event_source<int>( ctx );
    auto limit_range_begin = ureact::make_var( ctx, 4 );
    auto limit_range_size = ureact::make_var( ctx, 4 );

    // make deep dependent signals
    // so they are calculated later than not synced event filter
    auto limit_min = make_deeper( limit_range_begin );
    auto limit_max = make_deeper( limit_range_begin + limit_range_size - 1 );

    ureact::events<int> filtered;
    const auto in_range = []( auto i, int min, int max ) { return i >= min && i <= max; };

    SUBCASE( "Functional syntax" )
    {
        filtered = ureact::filter( src, with( limit_min, limit_max ), in_range );
    }
    // todo: Piped syntax is not yet supported for synced version
    //    SUBCASE( "Piped syntax" )
    //    {
    //        filtered = src | ureact::filter( with( limit_min, limit_max ), in_range );
    //    }

    const auto result = make_collector( filtered );

    // make not synced analog to show the difference
    const auto in_range_not_synced
        = [&]( auto i ) { return i >= limit_min.get() && i <= limit_max.get(); };
    ureact::events<int> filtered_not_synced = ureact::filter( src, in_range_not_synced );
    const auto result_not_synced = make_collector( filtered_not_synced );

    // pass 0-9 values into src
    for( int i = 0; i < 10; ++i )
        src << i;

    // change limits and pass the same values second time
    ctx.do_transaction( [&]() {
        for( int i = 0; i < 10; ++i )
            src << i;

        limit_range_begin <<= 1;
        limit_range_size <<= 3;
    } );

    // we expect only numbers in [limit_min, limit_max] range passed our filter
    // synced filtering performed only after new limit values are calculated
    const std::vector<int> expected = { /*first range*/ 4, 5, 6, 7, /*second range*/ 1, 2, 3 };
    CHECK( result.get() == expected );

    // we expect that second pass would use the old limit values because they are not recalculated yet
    const std::vector<int> expected_not_synced
        = { /*first range*/ 4, 5, 6, 7, /*second range*/ 4, 5, 6, 7 };
    CHECK( result_not_synced.get() == expected_not_synced );
}

// filters that take first N elements or skip first N elements
TEST_CASE( "TakeOrDropN" )
{
    ureact::context ctx;

    auto src = ureact::make_event_source<int>( ctx );
    ureact::events<int> first_n;
    ureact::events<int> without_first_n;

    SUBCASE( "Functional syntax" )
    {
        first_n = ureact::take( src, 5 );
        without_first_n = ureact::drop( src, 5 );
    }
    SUBCASE( "Piped syntax" )
    {
        first_n = src | ureact::take( 5 );
        without_first_n = src | ureact::drop( 5 );
    }

    const auto result_first_n = make_collector( first_n );
    const auto result_without_first_n = make_collector( without_first_n );

    // pass integers as events
    for( int i : { 0, 1, 2, 3, 4, 5, -1, 6, 7, 8, 9 } )
        src << i;

    // if we concatenate results of take(N) and drop(N) we receive original set of events
    const std::vector<int> expected_first_n = //
        { 0, 1, 2, 3, 4 };
    const std::vector<int> expected_without_first_n = //
        /*          */ { 5, -1, 6, 7, 8, 9 };
    CHECK( result_first_n.get() == expected_first_n );
    CHECK( result_without_first_n.get() == expected_without_first_n );
}

// take only first event from the source
TEST_CASE( "Once" )
{
    ureact::context ctx;

    const auto is_negative = [&]( auto i ) { return i < 0; };

    auto src = ureact::make_event_source<int>( ctx );
    ureact::events<int> negatives = ureact::filter( src, is_negative );
    ureact::events<int> first;
    ureact::events<int> first_negative;

    SUBCASE( "Functional syntax" )
    {
        first = ureact::once( src );
        first_negative = ureact::once( negatives );
    }
    SUBCASE( "Piped syntax" )
    {
        first = src | ureact::once();
        first_negative = negatives | ureact::once();
    }

    const auto result_first = make_collector( first );
    const auto result_first_negative = make_collector( first_negative );

    // pass integers as events
    for( int i : { 5, 1, 2, 4, -6, 0, -2 } )
        src << i;

    const std::vector<int> expected_first = { 5 };
    CHECK( result_first.get() == expected_first );

    const std::vector<int> expected_first_negative = { -6 };
    CHECK( result_first_negative.get() == expected_first_negative );
}

// filters that take first elements or skip first elements according to given predicate
TEST_CASE( "TakeOrDropWhile" )
{
    ureact::context ctx;

    auto src = ureact::make_event_source<int>( ctx );
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

    const auto result_before_negative = make_collector( before_negative );
    const auto result_from_negative = make_collector( from_negative );

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

    auto src = ureact::make_event_source<int>( ctx );
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
    // todo: Piped syntax is not yet supported for synced version
    //    SUBCASE( "Piped syntax" )
    //    {
    //        before_overflow = src | ureact::take_while( with(sum, blackjack), is_not_overflowed );
    //        from_overflow = src | ureact::drop_while( with(sum, blackjack), is_not_overflowed );
    //    }
    SUBCASE( "Calculated bool condition" )
    {
        before_overflow = ureact::take_while( src, with( overflowed ), is_not_overflowed_2 );
        from_overflow = ureact::drop_while( src, with( overflowed ), is_not_overflowed_2 );
    }

    const auto result_before_overflow = make_collector( before_overflow );
    const auto result_from_overflow = make_collector( from_overflow );

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

// demonstrate and test special filter function unique
TEST_CASE( "Unique" )
{
    ureact::context ctx;

    auto src = ureact::make_event_source<int>( ctx );
    ureact::events<int> uniq;

    SUBCASE( "Functional syntax" )
    {
        uniq = ureact::unique( src );
    }
    SUBCASE( "Piped syntax" )
    {
        uniq = src | ureact::unique();
    }

    const auto result = make_collector( uniq );

    // pass set containing several duplicate elements
    for( int i : { 1, 2, 1, 1, 3, 3, 3, 4, 5, 4 } )
        src << i;

    // expect removing consecutive (adjacent) duplicates
    const std::vector<int> expected = { 1, 2, 1, 3, 4, 5, 4 };
    CHECK( result.get() == expected );
}

// hold the last event value as a signal
TEST_CASE( "Hold" )
{
    ureact::context ctx;

    auto src = ureact::make_event_source<int>( ctx );
    ureact::signal<int> held;

    // there are two syntax variants, so we check them all using subcases
    SUBCASE( "Functional syntax" )
    {
        held = ureact::hold( src, -1 );
    }
    SUBCASE( "Piped syntax" )
    {
        held = src | ureact::hold( -1 );
    }

    int changes = 0;
    observe( held, [&changes]( const auto& ) { ++changes; } );

    // pass values into src
    for( int i : { 0, 0, 1, 1, 2, 3 } )
    {
        src << i;
        CHECK( held.get() == i );
    }

    CHECK( changes == 4 );
}

// sets signal value to value of other signal when event is received
TEST_CASE( "Snapshot" )
{
    ureact::context ctx;

    auto trigger = ureact::make_event_source( ctx );
    auto target = ureact::make_var<int>( ctx, -1 );
    ureact::signal<int> snap;

    // there are two syntax variants, so we check them all using subcases
    SUBCASE( "Functional syntax" )
    {
        snap = ureact::snapshot( trigger, target );
    }
    SUBCASE( "Piped syntax" )
    {
        snap = trigger | ureact::snapshot( target );
    }

    SUBCASE( "Trigger can be any type" )
    {
        snap = ureact::transform( trigger, []( ureact::token ) { return 1; } )
             | ureact::snapshot( target );
    }

    int changes = 0;
    observe( snap, [&changes]( const auto& ) { ++changes; } );

    // pass 0-9 values into src
    for( int i = 0; i < 10; ++i )
    {
        // ensure changing the target do not change the out
        target <<= i;
        assert( snap.get() != target.get() );

        // ensure emitting the trigger changes the out
        trigger();
        assert( snap.get() == i );

        // additionally trigger to ensure it does not add additional changes
        for( int j = 0; j < 3; ++j )
            trigger();
    }

    CHECK( changes == 10 );
}

// Emits value of target signal when event is received
TEST_CASE( "Pulse" )
{
    ureact::context ctx;

    auto trigger = ureact::make_event_source( ctx );
    auto target = ureact::make_var<int>( ctx, -1 );
    ureact::events<int> beat;

    // there are two syntax variants, so we check them all using subcases
    SUBCASE( "Functional syntax" )
    {
        beat = ureact::pulse( trigger, target );
    }
    SUBCASE( "Piped syntax" )
    {
        beat = trigger | ureact::pulse( target );
    }

    SUBCASE( "Trigger can be any type" )
    {
        beat = ureact::transform( trigger, []( ureact::token ) { return 1; } )
             | ureact::pulse( target );
    }

    auto result = make_collector( beat );

    target <<= 1;
    for( int i = 0; i < 2; ++i )
        trigger();

    ctx.do_transaction( [&]() {
        target <<= 6;
        for( int i = 0; i < 3; ++i )
            trigger();
    } );

    // we expect first value twice and second value thrice
    const std::vector<int> expected = { 1, 1, 6, 6, 6 };
    CHECK( result.get() == expected );
}

// Emits value changes of signal as events
TEST_CASE( "Monitor" )
{
    ureact::context ctx;

    auto src = ureact::make_var<int>( ctx, -1 );
    ureact::events<int> monitored;
    ureact::events<> changes;
    ureact::events<> changes_to_zero;

    // there are two syntax variants, so we check them all
    // using subcases
    SUBCASE( "Functional syntax" )
    {
        monitored = ureact::monitor( src );
        changes = ureact::changed( src );
        changes_to_zero = ureact::changed_to( src, 0 );
    }
    //    SUBCASE( "Piped syntax" )
    //    {
    //        // TODO: the is a problem with ambiguous operator|
    //        // maybe I need to create lambda wrapper called algorithm or something
    //        // to disable signal<S> | func(S) overload
    //        monitored = src | ureact::monitor();
    //        changes = src | ureact::changed();
    //        changes_to_zero = src | ureact::changed_to( 0 );
    //    }

    const auto result = make_collector( monitored );
    const auto changes_count = make_counter( changes );
    const auto changes_to_zero_count = make_counter( changes_to_zero );

    // pass values into src
    for( int i : { 0, 0, 0, 1, 1, 2, 3, 0 } )
        src <<= i;

    const std::vector<int> expected = { 0, 1, 2, 3, 0 };
    CHECK( result.get() == expected );
    CHECK( changes_count.get() == 5 );
    CHECK( changes_to_zero_count.get() == 2 );
}
