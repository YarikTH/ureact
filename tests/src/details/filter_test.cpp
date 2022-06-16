//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <doctest.h>

#include "ureact/ureact.hpp"

namespace
{

const auto always_true = []( auto&& ) { return true; };

// use fold expression to collect all received events into a vector
template <class T>
auto make_collector( T&& signal )
{
    const auto collector = []( int e, std::vector<int>& accum ) { accum.push_back( e ); };
    return ureact::fold( std::forward<T>( signal ), std::vector<int>{}, collector );
}

template <class T>
auto make_deeper( T&& signal )
{
    ureact::signal<int> result = signal;
    for( int i = 0; i < 100; ++i )
        result = +result;
    return result;
}

} // namespace

// filter only even integer events
// our check function depends only on event value
TEST_CASE( "Filter" )
{
    ureact::context ctx;

    auto src = ureact::make_event_source<int>( ctx );
    ureact::events<int> filtered;
    const auto is_even = []( auto i ) { return i % 2 == 0; };

    // there are two syntax variants and temporary event optimization, so we check them all
    // using subcases
    SUBCASE( "Functional syntax" )
    {
        filtered = ureact::filter( src, is_even );
    }
    SUBCASE( "Functional syntax on temporary" )
    {
        // filter returns temp_events
        // we use them to check filter overloads that receive temp_events rvalue
        // typically we don't need nor std::move nor naming temp events
        ureact::temp_events temp = ureact::filter( src, always_true );
        filtered = ureact::filter( std::move( temp ), is_even );
        // unfortunately there is no way to check if operator was actually stolen
    }

    SUBCASE( "Piped syntax" )
    {
        filtered = src | ureact::filter( is_even );
    }
    SUBCASE( "Piped syntax on temporary" )
    {
        ureact::temp_events temp = src | ureact::filter( always_true );
        filtered = std::move( temp ) | ureact::filter( is_even );
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
    SUBCASE( "Functional syntax on temporary" )
    {
        ureact::temp_events temp = ureact::filter( src, always_true );
        filtered = ureact::filter( std::move( temp ), with( limit_min, limit_max ), in_range );
        // unfortunately there is no way to check if operator was actually stolen
    }

    // todo: Piped syntax is not yet supported for synced version
    //    SUBCASE( "Piped syntax" )
    //    {
    //        filtered = src | ureact::filter( with( limit_min, limit_max ), in_range );
    //    }
    //    SUBCASE( "Piped syntax on temporary" )
    //    {
    //        ureact::temp_events temp = src | ureact::filter( always_true );
    //        filtered = std::move( temp ) | ureact::filter( with( limit_min, limit_max ), in_range );
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

// demonstrate and test special filter functions like take, drop and once
TEST_CASE( "SpecialFilterFunctions" )
{
    ureact::context ctx;

    auto src = ureact::make_event_source<int>( ctx );
    ureact::events<int> first;
    ureact::events<int> first_n;
    ureact::events<int> without_first_n;
    ureact::events<int> before_negative;
    ureact::events<int> from_negative;

    const auto is_not_negative = [&]( auto i ) { return i >= 0; };

    SUBCASE( "Functional syntax" )
    {
        first = ureact::once( src );
        first_n = ureact::take( src, 5 );
        without_first_n = ureact::drop( src, 5 );
        before_negative = ureact::take_while( src, is_not_negative );
        from_negative = ureact::drop_while( src, is_not_negative );
    }
    SUBCASE( "Piped syntax" )
    {
        first = src | ureact::once();
        first_n = src | ureact::take( 5 );
        without_first_n = src | ureact::drop( 5 );
        before_negative = src | ureact::take_while( is_not_negative );
        from_negative = src | ureact::drop_while( is_not_negative );
    }

    const auto result_first = make_collector( first );
    const auto result_first_n = make_collector( first_n );
    const auto result_without_first_n = make_collector( without_first_n );
    const auto result_before_negative = make_collector( before_negative );
    const auto result_from_negative = make_collector( from_negative );

    // pass integers as events
    for( int i : { 0, 1, 2, 3, 4, 5, -1, 6, 7, 8, 9 } )
        src << i;


    const std::vector<int> expected_first = //
        { 0 };
    CHECK( result_first.get() == expected_first );


    // if we concatenate results of take(N) and drop(N) we receive original vector
    const std::vector<int> expected_first_n = //
        { 0, 1, 2, 3, 4 };
    const std::vector<int> expected_without_first_n = //
        /*          */ { 5, -1, 6, 7, 8, 9 };
    CHECK( result_first_n.get() == expected_first_n );
    CHECK( result_without_first_n.get() == expected_without_first_n );


    // if we concatenate results of take_while(pred) and drop_while(pred) we receive original vector
    const std::vector<int> expected_before_negative = //
        { 0, 1, 2, 3, 4, 5 };
    const std::vector<int> expected_from_negative = //
        /*             */ { -1, 6, 7, 8, 9 };
    CHECK( result_before_negative.get() == expected_before_negative );
    CHECK( result_from_negative.get() == expected_from_negative );
}
