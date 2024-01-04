//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/tap.hpp"

#include "catch2_extra.hpp"
#include "ureact/events.hpp"
#include "ureact/signal.hpp"

TEST_CASE( "ureact::tap (signal<S>)" )
{
    ureact::context ctx;

    auto src = make_var( ctx, 1 );
    ureact::signal<int> result;

    std::vector<int> observed_src;

    auto collector = [&observed_src]( int x ) { //
        observed_src.push_back( x );
    };

    SECTION( "Functional syntax" )
    {
        result = ureact::tap( src, collector );
    }
    SECTION( "Piped syntax" )
    {
        result = src | ureact::tap( collector );
    }

    for( int i : { 0, 1, 2 } )
        src <<= i;

    CHECK( observed_src == std::vector<int>{ 0, 1, 2 } );
}

TEST_CASE( "ureact::tap (events<E>)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> result;

    std::vector<int> observed_src;

    auto collector = [&observed_src]( int x ) { //
        observed_src.push_back( x );
    };

    SECTION( "Functional syntax" )
    {
        result = ureact::tap( src, collector );
    }
    SECTION( "Piped syntax" )
    {
        result = src | ureact::tap( collector );
    }

    for( int i : { 0, 1, 2 } )
        src << i;

    CHECK( observed_src == std::vector<int>{ 0, 1, 2 } );
}

TEST_CASE( "ureact::tap (events<E>, synced)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto _2 = make_const( ctx, 2 );

    ureact::events<int> result;

    std::vector<int> observed_src_mult;

    auto collector = [&observed_src_mult]( int x, int mult ) { //
        observed_src_mult.push_back( x * mult );
    };

    SECTION( "Functional syntax" )
    {
        result = ureact::tap( src, with( _2 ), collector );
    }
    SECTION( "Piped syntax" )
    {
        result = src | ureact::tap( with( _2 ), collector );
    }

    for( int i : { 0, 1, 2 } )
        src << i;

    CHECK( observed_src_mult == std::vector<int>{ 0, 2, 4 } );
}

TEST_CASE( "ureact::tap (Observer Policy for signal<S>)" )
{
    ureact::context ctx;

    auto x = make_var( ctx, 42 );

    ureact::signal<int> obs0;
    ureact::signal<int> obs1;
    ureact::signal<int> obs2;

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
        obs0 = ureact::tap( x, on_value_change( observed_default ) );
        obs1 = ureact::tap(
            x, on_value_change( observed_skip_current ), ureact::observe_policy::skip_current );
        obs2 = ureact::tap(
            x, on_value_change( observed_notify_current ), ureact::observe_policy::notify_current );
    }
    SECTION( "Piped syntax" )
    {
        obs0 = x | ureact::tap( on_value_change( observed_default ) );
        obs1 = x
             | ureact::tap(
                 on_value_change( observed_skip_current ), ureact::observe_policy::skip_current );
        obs2 = x
             | ureact::tap( on_value_change( observed_notify_current ),
                 ureact::observe_policy::notify_current );
    }

    for( int i : { 1, 2 } )
        x <<= i;

    CHECK( observed_default == std::vector<int>{ 1, 2 } );
    CHECK( observed_skip_current == std::vector<int>{ 1, 2 } );
    CHECK( observed_notify_current == std::vector<int>{ 42, 1, 2 } );
}

TEST_CASE( "ureact::tap (Observer Policy and auto detach)" )
{
    ureact::context ctx;

    auto x = make_var( ctx, 42 );

    ureact::signal<int> obs1;
    ureact::signal<int> obs2;

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
        obs1 = ureact::tap(
            x, on_value_change( observed_skip_current ), ureact::observe_policy::skip_current );
        obs2 = ureact::tap(
            x, on_value_change( observed_notify_current ), ureact::observe_policy::notify_current );
    }
    SECTION( "Piped syntax" )
    {
        obs1 = x
             | ureact::tap(
                 on_value_change( observed_skip_current ), ureact::observe_policy::skip_current );
        obs2 = x
             | ureact::tap( on_value_change( observed_notify_current ),
                 ureact::observe_policy::notify_current );
    }

    for( int i : { 1, 2, 42 } )
        x <<= i;

    CHECK( observed_skip_current == std::vector<int>{ 1, 2, 42 } );
    CHECK( observed_notify_current == std::vector<int>{ 42 } );
}
