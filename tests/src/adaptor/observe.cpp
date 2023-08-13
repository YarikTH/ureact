//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/observe.hpp"

#include <list>
#include <ranges>
#include <sstream>
#include <vector>

#include "catch2_extra.hpp"
#include "ureact/events.hpp"
#include "ureact/signal.hpp"

// TODO: move here tests for ureact::observe

TEST_CASE( "ureact::observe (Observer Policy for signal<S>)" )
{
    ureact::context ctx;

    auto x = make_var( ctx, 42 );

    ureact::observer obs0;
    ureact::observer obs1;
    ureact::observer obs2;

    std::vector<int> observed_default;
    std::vector<int> observed_skip_current;
    std::vector<int> observed_notify_current;

    const auto on_value_change = []( std::vector<int>& out ) {
        return [&]( const int new_value ) { //
            out.push_back( new_value );
        };
    };

    SECTION( "Functional syntax" )
    {
        obs0 = ureact::observe( x, on_value_change( observed_default ) );
        obs1 = ureact::observe(
            x, on_value_change( observed_skip_current ), ureact::observe_policy::skip_current );
        obs2 = ureact::observe(
            x, on_value_change( observed_notify_current ), ureact::observe_policy::notify_current );
    }
    SECTION( "Piped syntax" )
    {
        obs0 = x | ureact::observe( on_value_change( observed_default ) );
        obs1 = x
             | ureact::observe(
                 on_value_change( observed_skip_current ), ureact::observe_policy::skip_current );
        obs2 = x
             | ureact::observe( on_value_change( observed_notify_current ),
                 ureact::observe_policy::notify_current );
    }

    for( int i : { 1, 2 } )
        x <<= i;

    CHECK( observed_default == std::vector<int>{ 1, 2 } );
    CHECK( observed_skip_current == std::vector<int>{ 1, 2 } );
    CHECK( observed_notify_current == std::vector<int>{ 42, 1, 2 } );
}

TEST_CASE( "ureact::observe (Observer Policy and auto detach)" )
{
    ureact::context ctx;

    auto x = make_var( ctx, 42 );

    ureact::observer obs1;
    ureact::observer obs2;

    std::vector<int> observed_skip_current;
    std::vector<int> observed_notify_current;

    const auto on_value_change = []( std::vector<int>& out ) {
        return [&]( const int new_value ) { //
            out.push_back( new_value );
            return new_value == 42 ? ureact::observer_action::stop_and_detach
                                   : ureact::observer_action::next;
        };
    };

    SECTION( "Functional syntax" )
    {
        obs1 = ureact::observe(
            x, on_value_change( observed_skip_current ), ureact::observe_policy::skip_current );
        obs2 = ureact::observe(
            x, on_value_change( observed_notify_current ), ureact::observe_policy::notify_current );
    }
    SECTION( "Piped syntax" )
    {
        obs1 = x
             | ureact::observe(
                 on_value_change( observed_skip_current ), ureact::observe_policy::skip_current );
        obs2 = x
             | ureact::observe( on_value_change( observed_notify_current ),
                 ureact::observe_policy::notify_current );
    }

    for( int i : { 1, 2, 42 } )
        x <<= i;

    CHECK( observed_skip_current == std::vector<int>{ 1, 2, 42 } );
    CHECK( observed_notify_current == std::vector<int>{ 42 } );
}

TEST_CASE( "ureact::observe (Output iterator support)" )
{
    ureact::context ctx;

    ureact::observer obs_printer;
    ureact::observer obs_collector;
    ureact::observer obs_collector_2;

    std::ostringstream os;
    std::vector<int> values;
    std::vector<int> values2( 6 );

    auto setup_observers = [&]( auto src ) {
        obs_printer = ureact::observe( src, std::ostream_iterator<int>( os, ", " ) );
        obs_collector = ureact::observe( src, std::back_inserter( values ) );
        obs_collector_2 = ureact::observe( src, values2.begin() + 1 );
    };

    SECTION( "signal<S>" )
    {
        auto x = ureact::make_var( ctx, -1 );

        setup_observers( x );

        for( int i : { 1, 2, -1, 42 } )
            x <<= i;
    }
    SECTION( "events<E>" )
    {
        auto src = ureact::make_source<int>( ctx );

        setup_observers( src );

        for( int i : { 1, 2, -1, 42 } )
            src << i;
    }

    CHECK( os.str() == "1, 2, -1, 42, " );
    CHECK( values == std::vector<int>{ 1, 2, -1, 42 } );
    CHECK( values2 == std::vector<int>{ 0, 1, 2, -1, 42, 0 } );
}

TEST_CASE( "ureact::observe (Output range support)" )
{
    ureact::context ctx;

    ureact::observer obs1;
    ureact::observer obs2;
    ureact::observer obs3;
    ureact::observer obs4;

    std::vector<int> values1( 2 );
    int values2[3] = {};
    std::list<int> values3( 5 );
    std::vector<int> values4;

    auto setup_observers = [&]( auto src ) {
        obs1 = ureact::observe( src, values1 );
        obs2 = ureact::observe( src, values2 );
        obs3 = ureact::observe(
            src, values3 | std::views::reverse | std::views::drop( 1 ) | std::views::take( 3 ) );
        obs4 = ureact::observe( src, values4 );
    };
    
//    std::begin(values3 | std::views::reverse | std::views::drop( 1 ) | std::views::take( 3 ));
//    
    //*std::begin(values3 | std::views::reverse | std::views::drop( 1 ) | std::views::take( 3 ))++ = 4;

    SECTION( "signal<S>" )
    {
        auto x = ureact::make_var( ctx, -1 );

        setup_observers( x );

        for( int i : { 1, 2, -1, 42 } )
            x <<= i;
    }
    SECTION( "events<E>" )
    {
        auto src = ureact::make_source<int>( ctx );

        setup_observers( src );

        for( int i : { 1, 2, -1, 42 } )
            src << i;
    }

    CHECK( values1 == std::vector<int>{ 1, 2 } );
    CHECK(
        std::vector( std::begin( values2 ), std::end( values2 ) ) == std::vector<int>{ 1, 2, -1 } );
    CHECK( values3 == std::list<int>{ 0, -1, 2, 1, 0 } );
    CHECK( values4.empty() );
}
