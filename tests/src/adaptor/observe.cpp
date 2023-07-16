//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/observe.hpp"

#include "catch2_extra.hpp"
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
