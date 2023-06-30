//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
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

    CHECK( result.equal_to( src ) );

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

    CHECK( result.equal_to( src ) );

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

    CHECK( result.equal_to( src ) );

    for( int i : { 0, 1, 2 } )
        src << i;

    CHECK( observed_src_mult == std::vector<int>{ 0, 2, 4 } );
}