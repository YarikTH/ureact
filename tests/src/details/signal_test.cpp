//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <queue>

#include <doctest.h>

#include "ureact/ureact.hpp"

namespace
{

int add( int a, int b )
{
    return a + b;
}
float halve( int a )
{
    return static_cast<float>( a ) / 2.0f;
}
float multiply( float a, float b )
{
    return a * b;
}

} // namespace

TEST_SUITE_BEGIN( "SignalTest" );

TEST_CASE( "MakeVars" )
{
    ureact::context ctx;

    auto v1 = make_value( ctx, 1 );
    auto v2 = make_value( ctx, 2 );
    auto v3 = make_value( ctx, 3 );
    auto v4 = make_value( ctx, 4 );

    CHECK( v1.get() == 1 );
    CHECK( v2.get() == 2 );
    CHECK( v3.get() == 3 );
    CHECK( v4.get() == 4 );

    v1 <<= 10;
    v2 <<= 20;
    v3 <<= 30;
    v4 <<= 40;

    CHECK( v1.get() == 10 );
    CHECK( v2.get() == 20 );
    CHECK( v3.get() == 30 );
    CHECK( v4.get() == 40 );
}

TEST_CASE( "Signals1" )
{
    ureact::context ctx;

    auto v1 = make_value( ctx, 1 );
    auto v2 = make_value( ctx, 2 );
    auto v3 = make_value( ctx, 3 );
    auto v4 = make_value( ctx, 4 );

    auto s1 = make_signal( with( v1, v2 ), []( int a, int b ) { return a + b; } );

    auto s2 = make_signal( with( v3, v4 ), []( int a, int b ) { return a + b; } );

    auto s3 = s1 + s2;

    CHECK( s1.get() == 3 );
    CHECK( s2.get() == 7 );
    CHECK( s3.get() == 10 );

    v1 <<= 10;
    v2 <<= 20;
    v3 <<= 30;
    v4 <<= 40;

    CHECK( s1.get() == 30 );
    CHECK( s2.get() == 70 );
    CHECK( s3.get() == 100 );

    CHECK( ureact::is_signal<decltype( v1 )>::value );

    CHECK( ureact::is_signal<decltype( s1 )>::value );

    CHECK( ureact::is_signal<decltype( s2 )>::value );

    CHECK_FALSE( ureact::is_signal<decltype( 10 )>::value );
}

TEST_CASE( "Signals2" )
{
    ureact::context ctx;

    auto a1 = make_value( ctx, 1 );
    auto a2 = make_value( ctx, 1 );

    auto b1 = a1 + 0;
    auto b2 = a1 + a2;
    auto b3 = a2 + 0;

    auto c1 = b1 + b2;
    auto c2 = b2 + b3;

    auto result = c1 + c2;

    int observeCount = 0;

    observe( result, [&observeCount]( int v ) {
        observeCount++;
        if( observeCount == 1 )
            CHECK( v == 9 );
        else
            CHECK( v == 12 );
    } );

    CHECK( a1.get() == 1 );
    CHECK( a2.get() == 1 );

    CHECK( b1.get() == 1 );
    CHECK( b2.get() == 2 );
    CHECK( b3.get() == 1 );

    CHECK( c1.get() == 3 );
    CHECK( c2.get() == 3 );

    CHECK( result.get() == 6 );

    a1 <<= 2;

    CHECK( observeCount == 1 );

    CHECK( a1.get() == 2 );
    CHECK( a2.get() == 1 );

    CHECK( b1.get() == 2 );
    CHECK( b2.get() == 3 );
    CHECK( b3.get() == 1 );

    CHECK( c1.get() == 5 );
    CHECK( c2.get() == 4 );

    CHECK( result.get() == 9 );

    a2 <<= 2;

    CHECK( observeCount == 2 );

    CHECK( a1.get() == 2 );
    CHECK( a2.get() == 2 );

    CHECK( b1.get() == 2 );
    CHECK( b2.get() == 4 );
    CHECK( b3.get() == 2 );

    CHECK( c1.get() == 6 );
    CHECK( c2.get() == 6 );

    CHECK( result.get() == 12 );
}

TEST_CASE( "Signals3" )
{
    ureact::context ctx;

    auto a1 = make_value( ctx, 1 );
    auto a2 = make_value( ctx, 1 );

    auto b1 = a1 + 0;
    auto b2 = a1 + a2;
    auto b3 = a2 + 0;

    auto c1 = b1 + b2;
    auto c2 = b2 + b3;

    auto result = c1 + c2;

    int observeCount = 0;

    observe( result, [&observeCount]( int v ) {
        observeCount++;
        CHECK( v == 12 );
    } );

    CHECK( a1.get() == 1 );
    CHECK( a2.get() == 1 );

    CHECK( b1.get() == 1 );
    CHECK( b2.get() == 2 );
    CHECK( b3.get() == 1 );

    CHECK( c1.get() == 3 );
    CHECK( c2.get() == 3 );

    CHECK( result.get() == 6 );

    ctx.do_transaction( [&] {
        a1 <<= 2;
        a2 <<= 2;
    } );

    CHECK( observeCount == 1 );

    CHECK( a1.get() == 2 );
    CHECK( a2.get() == 2 );

    CHECK( b1.get() == 2 );
    CHECK( b2.get() == 4 );
    CHECK( b3.get() == 2 );

    CHECK( c1.get() == 6 );
    CHECK( c2.get() == 6 );

    CHECK( result.get() == 12 );
}

TEST_CASE( "Signals4" )
{
    ureact::context ctx;

    auto a1 = make_value( ctx, 1 );
    auto a2 = make_value( ctx, 1 );

    auto b1 = a1 + a2;
    auto b2 = b1 + a2;

    CHECK( a1.get() == 1 );
    CHECK( a2.get() == 1 );

    CHECK( b1.get() == 2 );
    CHECK( b2.get() == 3 );

    a1 <<= 10;

    CHECK( a1.get() == 10 );
    CHECK( a2.get() == 1 );

    CHECK( b1.get() == 11 );
    CHECK( b2.get() == 12 );
}

TEST_CASE( "FunctionBind1" )
{
    ureact::context ctx;

    auto v1 = make_value( ctx, 2 );
    auto v2 = make_value( ctx, 30 );
    auto v3 = make_value( ctx, 10 );

    auto signal = ( v1, v2, v3 ) | [=]( int a, int b, int c ) -> int { return a * b * c; };

    CHECK( signal.get() == 600 );
    v3 <<= 100;
    CHECK( signal.get() == 6000 );
}

TEST_CASE( "FunctionBind2" )
{
    ureact::context ctx;

    auto a = make_value( ctx, 1 );
    auto b = make_value( ctx, 1 );

    auto c = ( ( a + b ), ( a + 100 ) ) | &add;
    auto d = c | &halve;
    auto e = ( d, d ) | &multiply;
    auto f = -e + 100.f;

    CHECK( c.get() == 103 );
    CHECK( d.get() == 51.5f );
    CHECK( e.get() == 2652.25f );
    CHECK( f.get() == -2552.25f );

    a <<= 10;

    CHECK( c.get() == 121 );
    CHECK( d.get() == 60.5f );
    CHECK( e.get() == 3660.25f );
    CHECK( f.get() == -3560.25f );
}

TEST_CASE( "Compose signals" )
{
    ureact::context ctx;

    auto a = make_value( ctx, 1 );
    auto b = make_value( ctx, 1 );
    auto inverse_value = []( int i ) { return -i; };
    auto double_value = []( int i ) { return i * 2; };
    auto sum_values = []( int i, int j ) { return i + j; };

    // x = a * 2 * 2 * 2 == a * 8
    auto x = a | double_value | double_value | double_value;

    // y = (2 * a + (-1) * b) * 2
    auto y = ( a | double_value, b | inverse_value ) | sum_values | double_value;

    CHECK( x.get() == 1 * 2 * 2 * 2 );
    CHECK( y.get() == ( 2 * 1 + ( -1 ) * 1 ) * 2 );

    a <<= 7;
    b <<= 5;

    CHECK( x.get() == 7 * 2 * 2 * 2 );
    CHECK( y.get() == ( 2 * 7 + ( -1 ) * 5 ) * 2 );
}

TEST_CASE( "Flatten1" )
{
    ureact::context ctx;

    auto inner1 = make_value( ctx, 123 );
    auto inner2 = make_value( ctx, 789 );

    auto outer = make_value( ctx, inner1 );

    auto flattened = flatten( outer );

    std::queue<int> results;

    observe( flattened, [&]( int v ) { results.push( v ); } );

    CHECK( outer.get().equals( inner1 ) );
    CHECK( flattened.get() == 123 );

    inner1 <<= 456;

    CHECK( flattened.get() == 456 );

    CHECK( results.front() == 456 );
    results.pop();
    CHECK( results.empty() );

    outer <<= inner2;

    CHECK( outer.get().equals( inner2 ) );
    CHECK( flattened.get() == 789 );

    CHECK( results.front() == 789 );
    results.pop();
    CHECK( results.empty() );
}

TEST_CASE( "Flatten2" )
{
    ureact::context ctx;

    auto a0 = make_value( ctx, 100 );

    auto inner1 = make_value( ctx, 200 );

    auto a1 = make_value( ctx, 300 );
    auto a2 = a1 + 0;
    auto a3 = a2 + 0;
    auto a4 = a3 + 0;
    auto a5 = a4 + 0;
    auto a6 = a5 + 0;
    auto inner2 = a6 + 0;

    CHECK( inner1.get() == 200 );
    CHECK( inner2.get() == 300 );

    auto outer = make_value( ctx, inner1 );

    auto flattened = flatten( outer );

    CHECK( flattened.get() == 200 );

    int observeCount = 0;

    observe( flattened, [&observeCount]( int /*unused*/ ) { observeCount++; } );

    auto o1 = a0 + flattened;
    auto o2 = o1 + 0;
    auto o3 = o2 + 0;
    auto result = o3 + 0;

    CHECK( result.get() == 100 + 200 );

    inner1 <<= 1234;

    CHECK( result.get() == 100 + 1234 );
    CHECK( observeCount == 1 );

    outer <<= inner2;

    CHECK( result.get() == 100 + 300 );
    CHECK( observeCount == 2 );

    ctx.do_transaction( [&] {
        a0 <<= 5000;
        a1 <<= 6000;
    } );

    CHECK( result.get() == 5000 + 6000 );
    CHECK( observeCount == 3 );
}

TEST_CASE( "Flatten3" )
{
    ureact::context ctx;

    auto inner1 = make_value( ctx, 10 );

    auto a1 = make_value( ctx, 20 );
    auto a2 = a1 + 0;
    auto a3 = a2 + 0;
    auto inner2 = a3 + 0;

    auto outer = make_value( ctx, inner1 );

    auto a0 = make_value( ctx, 30 );

    auto flattened = flatten( outer );

    int observeCount = 0;

    observe( flattened, [&observeCount]( int /*unused*/ ) { observeCount++; } );

    auto result = flattened + a0;

    CHECK( result.get() == 10 + 30 );
    CHECK( observeCount == 0 );

    ctx.do_transaction( [&] {
        inner1 <<= 1000;
        a0 <<= 200000;
        a1 <<= 50000;
        outer <<= inner2;
    } );

    CHECK( result.get() == 50000 + 200000 );
    CHECK( observeCount == 1 );

    ctx.do_transaction( [&] {
        a0 <<= 667;
        a1 <<= 776;
    } );

    CHECK( result.get() == 776 + 667 );
    CHECK( observeCount == 2 );

    ctx.do_transaction( [&] {
        inner1 <<= 999;
        a0 <<= 888;
    } );

    CHECK( result.get() == 776 + 888 );
    CHECK( observeCount == 2 );
}

TEST_CASE( "Flatten4" )
{
    ureact::context ctx;

    std::vector<int> results;

    auto a1 = make_value( ctx, 100 );
    auto inner1 = a1 + 0;

    auto a2 = make_value( ctx, 200 );
    auto inner2 = a2;

    auto a3 = make_value( ctx, 200 );

    auto outer = make_value( ctx, inner1 );

    auto flattened = flatten( outer );

    auto result = flattened + a3;

    observe( result, [&]( int v ) { results.push_back( v ); } );

    ctx.do_transaction( [&] {
        a3 <<= 400;
        outer <<= inner2;
    } );

    CHECK( results.size() == 1 );

    CHECK( std::find( results.begin(), results.end(), 600 ) != results.end() );
}

TEST_CASE( "Flatten5" )
{
    ureact::context ctx;

    auto inner1 = make_value( ctx, 123 );
    auto inner2 = make_value( ctx, 123 );

    auto outer = make_value( ctx, inner1 );

    auto flattened = flatten( outer );

    std::queue<int> results;

    observe( flattened, [&]( int v ) { results.push( v ); } );

    CHECK( outer.get().equals( inner1 ) );
    CHECK( flattened.get() == 123 );

    CHECK( results.empty() );

    outer <<= inner2;

    CHECK( outer.get().equals( inner2 ) );
    CHECK( flattened.get() == 123 );

    // flattened observer shouldn't trigger if value isn't changed
    CHECK( results.empty() );
}

TEST_CASE( "Modify1" )
{
    ureact::context ctx;

    auto v = make_value( ctx, std::vector<int>{} );

    int obsCount = 0;

    observe( v, [&]( const std::vector<int>& v_ ) {
        REQUIRE( v_.size() == 3 );
        CHECK( v_[0] == 30 );
        CHECK( v_[1] == 50 );
        CHECK( v_[2] == 70 );

        obsCount++;
    } );

    v.modify( []( std::vector<int>& v_ ) {
        v_.push_back( 30 );
        v_.push_back( 50 );
        v_.push_back( 70 );
    } );

    CHECK( obsCount == 1 );
}

TEST_CASE( "Modify2" )
{
    ureact::context ctx;

    auto v = make_value( ctx, std::vector<int>{} );

    int obsCount = 0;

    observe( v, [&]( const std::vector<int>& v_ ) {
        REQUIRE( v_.size() == 3 );
        CHECK( v_[0] == 30 );
        CHECK( v_[1] == 50 );
        CHECK( v_[2] == 70 );

        obsCount++;
    } );

    ctx.do_transaction( [&] {
        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 30 ); } );

        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 50 ); } );

        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 70 ); } );
    } );

    CHECK( obsCount == 1 );
}

TEST_CASE( "Modify3" )
{
    ureact::context ctx;

    auto value = make_value( ctx, std::vector<int>{} );

    int obsCount = 0;

    observe( value, [&]( const std::vector<int>& v ) {
        CHECK( v[0] == 30 );
        CHECK( v[1] == 50 );
        CHECK( v[2] == 70 );

        obsCount++;
    } );

    ctx.do_transaction( [&] {
        value.set( std::vector<int>{ 30, 50 } );

        value.modify( []( std::vector<int>& v ) { v.push_back( 70 ); } );
    } );

    CHECK( obsCount == 1 );
}

TEST_CASE( "Recursive transactions" )
{
    ureact::context ctx;

    auto v1 = make_value( ctx, 1 );

    int observeCount = 0;

    observe( v1, [&observeCount]( int /*v*/ ) { observeCount++; } );

    ctx.do_transaction( [&] {
        v1 <<= 7;

        ctx.do_transaction( [&] { v1 <<= 4; } );

        v1 <<= 1;
        v1 <<= 2;
    } );

    CHECK( observeCount == 1 );
}

TEST_SUITE_END();
