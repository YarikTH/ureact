//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "doctest_extra.h"
#include "ureact/lift.hpp"
#include "ureact/observe.hpp"
#include "ureact/ureact.hpp"

// TODO: check type traits for signals
// * signal<S>
// * var_signal<S>
// * temp_signal<S, ...>
// * signal_pack

// TODO: remake MakeVars
// TODO: remake SignalsN
// TODO: remake ModifyN

TEST_CASE( "SignalConstruction" )
{
    ureact::context ctx;

    // default constructed signal isn't linked to a reactive node, thus
    // can't be used for anything but for following assignment
    SUBCASE( "default constructed" )
    {
        ureact::signal<int> null_signal;
        CHECK_FALSE( null_signal.is_valid() );
    }

    // signal can be created via object slicing from var_signal object
    SUBCASE( "slicing" )
    {
        auto var = ureact::make_var<int>( ctx, 5 );
        ureact::signal sig = var;

        CHECK( sig.is_valid() );
    }

    // signal can be created via make_const call
    SUBCASE( "make_const" )
    {
        ureact::signal sig = ureact::make_const<int>( ctx, 5 );

        CHECK( sig.is_valid() );
    }

    // signal can be created using various algorithms (in particular using overloaded operator)
    SUBCASE( "algorithm" )
    {
        auto var = ureact::make_var<int>( ctx, 5 );
        ureact::signal sig = lift( var, []( int a ) { return a; } );

        CHECK( sig.is_valid() );
    }

    // copy and move construction of signal
    SUBCASE( "copy and move constructed" )
    {
        ureact::signal<int> sig = ureact::make_var<int>( ctx, 42 );
        CHECK( sig.is_valid() );

        SUBCASE( "copy constructed" )
        {
            ureact::signal sig_copy = sig;
            CHECK( sig_copy.is_valid() );
            CHECK( sig.is_valid() );
        }

        SUBCASE( "move constructed" )
        {
            ureact::signal sig_move = std::move( sig );
            CHECK( sig_move.is_valid() );
            CHECK_FALSE( sig.is_valid() );
        }
    }
}

TEST_CASE( "SignalAssignmentConstruction" )
{
    ureact::context ctx;

    ureact::signal<int> src = ureact::make_var<int>( ctx, 42 );
    CHECK( src.is_valid() );

    SUBCASE( "copy assignment" )
    {
        ureact::signal<int> src_copy;
        CHECK_FALSE( src_copy.is_valid() );

        src_copy = src;
        CHECK( src_copy.is_valid() );
        CHECK( src.is_valid() );
        CHECK( src_copy.equals( src ) );
    }

    SUBCASE( "move assignment" )
    {
        ureact::signal<int> src_move;
        CHECK_FALSE( src_move.is_valid() );

        src_move = std::move( src );
        CHECK( src_move.is_valid() );
        CHECK_FALSE( src.is_valid() );
    }
}

TEST_CASE( "VarSignalConstruction" )
{
    ureact::context ctx;

    // default constructed var_signal isn't linked to a reactive node, thus
    // can't be used for anything but for following assignment
    SUBCASE( "default constructed" )
    {
        ureact::var_signal<int> null_var;
        CHECK_FALSE( null_var.is_valid() );
    }

    // var_signal can be created via free function semantically close to std::make_shared
    // Signal value type S can be specified explicitly, but doesn't have to.
    SUBCASE( "make_var()" )
    {
        auto var = ureact::make_var( ctx, 8 );
        CHECK( var.is_valid() );
    }
    SUBCASE( "make_var<T>()" )
    {
        auto var = ureact::make_var<float>( ctx, 6 );
        CHECK( var.is_valid() );
    }

    // var_signal can be created using constructor receiving context reference and value
    SUBCASE( "fully constructed" )
    {
        ureact::var_signal src{ ctx, 3 };
        CHECK( src.is_valid() );
    }

    // var_signal can be created using constructor receiving context reference
    // in the form of AAA
    SUBCASE( "fully constructed AAA" )
    {
        auto src = ureact::var_signal{ ctx, 1 };
        CHECK( src.is_valid() );
    }

    // copy and move construction of var_signal
    SUBCASE( "copy and move constructed" )
    {
        ureact::var_signal<int> src{ ctx, 0 };
        CHECK( src.is_valid() );

        SUBCASE( "copy constructed" )
        {
            ureact::var_signal<int> src_copy = src;
            CHECK( src_copy.is_valid() );
            CHECK( src.is_valid() );
            CHECK( src_copy.equals( src ) );
        }

        SUBCASE( "move constructed" )
        {
            ureact::var_signal<int> src_move = std::move( src );
            CHECK( src_move.is_valid() );
            CHECK_FALSE( src.is_valid() );
        }
    }
}

TEST_CASE( "VarSignalAssignmentConstruction" )
{
    ureact::context ctx;

    ureact::var_signal<int> src{ ctx, -1 };
    CHECK( src.is_valid() );

    SUBCASE( "copy assignment" )
    {
        ureact::var_signal<int> src_copy;
        CHECK_FALSE( src_copy.is_valid() );

        src_copy = src;
        CHECK( src_copy.is_valid() );
        CHECK( src.is_valid() );
        CHECK( src_copy.equals( src ) );
    }

    SUBCASE( "move assignment" )
    {
        ureact::var_signal<int> src_move;
        CHECK_FALSE( src_move.is_valid() );

        src_move = std::move( src );
        CHECK( src_move.is_valid() );
        CHECK_FALSE( src.is_valid() );
    }
}

// Signal has shared_ptr semantics. They are literally shared_ptr to reactive node
// that does all the work and form dependency tree
TEST_CASE( "SignalSmartPointerSemantics" )
{
    ureact::context ctx;

    ureact::var_signal<int> x{ ctx, 1 };

    auto a = x * 2;

    auto result_x2 = +a;

    //       x       //
    //       |       //
    //    a(x * 2)   //
    //       |       //
    //   result_x2   //

    // reassigning of 'a' doesn't affect result_x2, because it depends not on
    // 'a' itself, but on reactive node it pointed before
    a = x * 3;

    auto result_x3 = +a;

    //                x                 //
    //          /           \           //
    //   a(x * 2)           a(x * 3)    //
    //       |                 |        //
    //  result_x2          result_x3    //

    CHECK( result_x2.get() == 2 );
    CHECK( result_x3.get() == 3 );

    x <<= 2;

    CHECK( result_x2.get() == 4 );
    CHECK( result_x3.get() == 6 );
}

TEST_CASE( "SignalGetValue" )
{
    ureact::context ctx;

    ureact::var_signal<int> src{ ctx, 1 };
    const ureact::signal<int> sig = +src;

    CHECK( sig.get() == 1 ); // get method
    CHECK( sig() == 1 );     // function object version

    src.set( 7 );

    CHECK( sig.get() == 7 );
    CHECK( sig() == 7 );
}

TEST_CASE( "VarSignalSetValue" )
{
    ureact::context ctx;

    ureact::var_signal<int> src{ ctx, 1 };

    CHECK( src.get() == 1 );

    int new_value{ 5 };

    SUBCASE( "set method L-value" )
    {
        src.set( new_value );
    }
    SUBCASE( "set method R-value" )
    {
        src.set( 5 );
    }
    SUBCASE( "set operator L-value" )
    {
        src <<= new_value;
    }
    SUBCASE( "set operator R-value" )
    {
        src <<= 5;
    }

    CHECK( src.get() == 5 );
}

TEST_CASE( "VarSignalModifyValue" )
{
    ureact::context ctx;

    ureact::var_signal<int> src{ ctx, 9 };

    CHECK( src.get() == 9 );

    auto change_int_to_5 = []( int& v ) { v = 4; };

    SUBCASE( "modify method" )
    {
        src.modify( change_int_to_5 );
    }
    SUBCASE( "modify operator" )
    {
        src <<= change_int_to_5;
    }

    CHECK( src.get() == 4 );
}

TEST_CASE( "MakeVars" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 1 );
    auto v2 = make_var( ctx, 2 );
    auto v3 = make_var( ctx, 3 );
    auto v4 = make_var( ctx, 4 );

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

    auto v1 = make_var( ctx, 1 );
    auto v2 = make_var( ctx, 2 );
    auto v3 = make_var( ctx, 3 );
    auto v4 = make_var( ctx, 4 );

    auto s1 = lift( with( v1, v2 ), []( int a, int b ) { return a + b; } );

    auto s2 = lift( with( v3, v4 ), []( int a, int b ) { return a + b; } );

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

    auto a1 = make_var( ctx, 1 );
    auto a2 = make_var( ctx, 1 );

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

    auto a1 = make_var( ctx, 1 );
    auto a2 = make_var( ctx, 1 );

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

    do_transaction( ctx, [&] {
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

    auto a1 = make_var( ctx, 1 );
    auto a2 = make_var( ctx, 1 );

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

    auto v1 = make_var( ctx, 2 );
    auto v2 = make_var( ctx, 30 );
    auto v3 = make_var( ctx, 10 );

    auto signal = with( v1, v2, v3 )
                | ureact::lift( [=]( int a, int b, int c ) -> int { return a * b * c; } );

    CHECK( signal.get() == 600 );
    v3 <<= 100;
    CHECK( signal.get() == 6000 );
}

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

TEST_CASE( "FunctionBind2" )
{
    ureact::context ctx;

    auto a = make_var( ctx, 1 );
    auto b = make_var( ctx, 1 );

    auto c = with( a + b, a + 100 ) | ureact::lift( &add );
    auto d = c | ureact::lift( &halve );
    auto e = with( d, d ) | ureact::lift( &multiply );
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

TEST_CASE( "Modify1" )
{
    ureact::context ctx;

    auto v = make_var( ctx, std::vector<int>{} );

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

    auto v = make_var( ctx, std::vector<int>{} );

    int obsCount = 0;

    observe( v, [&]( const std::vector<int>& v_ ) {
        REQUIRE( v_.size() == 3 );
        CHECK( v_[0] == 30 );
        CHECK( v_[1] == 50 );
        CHECK( v_[2] == 70 );

        obsCount++;
    } );

    do_transaction( ctx, [&] {
        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 30 ); } );

        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 50 ); } );

        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 70 ); } );
    } );

    CHECK( obsCount == 1 );
}

TEST_CASE( "Modify3" )
{
    ureact::context ctx;

    auto value = make_var( ctx, std::vector<int>{} );

    int obsCount = 0;

    observe( value, [&]( const std::vector<int>& v ) {
        CHECK( v[0] == 30 );
        CHECK( v[1] == 50 );
        CHECK( v[2] == 70 );

        obsCount++;
    } );

    do_transaction( ctx, [&] {
        value.set( std::vector<int>{ 30, 50 } );

        value.modify( []( std::vector<int>& v ) { v.push_back( 70 ); } );
    } );

    CHECK( obsCount == 1 );
}

TEST_CASE( "Reactive class members" )
{
    ureact::context ctx;

    class Shape
    {
    public:
        explicit Shape( ureact::context& ctx )
            : width( make_var( ctx, 0 ) )
            , height( make_var( ctx, 0 ) )
            , size( width * height )
        {}

        ureact::var_signal<int> width;
        ureact::var_signal<int> height;

        ureact::signal<int> size;
    };

    Shape my_shape( ctx );

    CHECK( my_shape.width.get() == 0 );
    CHECK( my_shape.height.get() == 0 );
    CHECK( my_shape.size.get() == 0 );

    std::vector<int> size_values;

    auto on_size_value_change = [&]( const int new_value ) { size_values.push_back( new_value ); };

    observe( my_shape.size, on_size_value_change );

    CHECK( size_values == std::vector<int>{} );

    // Do transaction to change width and height in single step
    do_transaction( ctx, [&] {
        my_shape.width <<= 4;
        my_shape.height <<= 4;
    } );

    CHECK( size_values == std::vector<int>{ 16 } );

    CHECK( my_shape.width.get() == 4 );
    CHECK( my_shape.height.get() == 4 );
    CHECK( my_shape.size.get() == 16 );
}

TEST_CASE( "Hello World" )
{
    ureact::context ctx;

    // make_var is available as a free function and as context's member function
    ureact::var_signal<std::string> firstWord = make_var( ctx, std::string( "Change" ) );
    ureact::var_signal<std::string> secondWord = make_var( ctx, std::string( "me!" ) );

    ureact::signal<std::string> bothWords;

    auto concatFunc = []( const std::string& first, const std::string& second ) -> std::string {
        return first + " " + second;
    };

    // Several alternative variants that do exactly the same
    SUBCASE( "using overloaded operators" )
    {
        bothWords = firstWord + " " + secondWord;
    }

    SUBCASE( "using lift()" )
    {
        bothWords = lift( with( firstWord, secondWord ), concatFunc );
    }

    SUBCASE( "operators , and |" )
    {
        // operator "|" can be used instead of lift()
        bothWords = with( firstWord, secondWord ) | ureact::lift( concatFunc );
    }

    // Imperative value access
    CHECK( bothWords.get() == "Change me!" );

    // Imperative value change
    firstWord <<= std::string( "Hello" );

    CHECK( bothWords.get() == "Hello me!" );

    secondWord <<= std::string( "World!" );

    CHECK( bothWords.get() == "Hello World!" );
}

TEST_CASE( "Modifying signal values in place" )
{
    ureact::context ctx;

    ureact::var_signal<std::vector<std::string>> data = make_var( ctx, std::vector<std::string>{} );

    CHECK( data.get() == std::vector<std::string>{} );

    data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "Hello" ); } );

    CHECK( data.get() == std::vector<std::string>{ "Hello" } );

    data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "World!" ); } );

    CHECK( data.get() == std::vector<std::string>{ "Hello", "World!" } );
}

TEST_CASE( "Changing multiple inputs" )
{
    ureact::context ctx;

    ureact::var_signal<int> a = make_var( ctx, 1 );
    ureact::var_signal<int> b = make_var( ctx, 1 );

    ureact::signal<int> x = a + b;
    ureact::signal<int> y = a + b;
    ureact::signal<int> z = x + y; // 2*(a+b)

    //  a b  //
    //  /X\  //
    // x   y //
    //  \ /  //
    //   z   //

    std::vector<int> z_values;
    observe( z, [&]( int new_value ) { z_values.push_back( new_value ); } );

    CHECK( z.get() == 4 );
    CHECK( z_values == std::vector<int>{} );

    a <<= 2; // z is changed to 6
    b <<= 2; // z is changed to 8

    do_transaction( ctx, [&] {
        a <<= 4;
        b <<= 4;
    } ); // z is changed to 16

    CHECK( z_values == std::vector<int>{ 6, 8, 16 } );
}

TEST_CASE( "Reacting to value changes" )
{
    ureact::context ctx;

    ureact::var_signal<int> x = make_var( ctx, 1 );
    ureact::signal<int> xAbs = lift( x, []( int value ) { return abs( value ); } );

    std::vector<int> xAbs_values;
    observe( xAbs, [&]( int new_value ) { xAbs_values.push_back( new_value ); } );

    CHECK( xAbs.get() == 1 );
    CHECK( xAbs_values == std::vector<int>{} );

    x <<= 2;  // xAbs is changed to 2
    x <<= -3; // xAbs is changed to 3
    x <<= 3;  // no output, xAbs is still 3

    do_transaction( ctx, [&] {
        x <<= 4;
        x <<= -2;
        x <<= 3;
    } ); // no output, result value of xAbs is still 3

    CHECK( xAbs_values == std::vector<int>{ 2, 3 } );
}
