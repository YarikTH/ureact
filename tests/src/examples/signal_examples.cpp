//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

TEST_SUITE( "Examples" )
{
    TEST_CASE( "Hello World" )
    {
        ureact::context ctx;

        // make_var is available as a free function and as context's member function
        ureact::var_signal<std::string> firstWord = make_var( ctx, std::string( "Change" ) );
        ureact::var_signal<std::string> secondWord = ctx.make_var( std::string( "me!" ) );

        ureact::signal<std::string> bothWords;

        auto concatFunc = []( const std::string& first, const std::string& second ) -> std::string {
            return first + " " + second;
        };

        // Several alternative variants that do exactly the same
        SUBCASE( "using overloaded operators" )
        {
            bothWords = firstWord + " " + secondWord;
        }

        SUBCASE( "using make_signal()" )
        {
            bothWords = make_signal( with( firstWord, secondWord ), concatFunc );
        }

        SUBCASE( "operators , and |" )
        {
            // operator "," can be used instead of function with()
            // operator "|" can be used instead of make_signal()
            bothWords = /*with*/ ( firstWord, secondWord ) | concatFunc;
        }

        // Imperative value access
        CHECK( bothWords.value() == "Change me!" );

        // Imperative value change
        firstWord <<= std::string( "Hello" );

        CHECK( bothWords.value() == "Hello me!" );

        secondWord <<= std::string( "World!" );

        CHECK( bothWords.value() == "Hello World!" );
    }

    TEST_CASE( "Modifying signal values in place" )
    {
        ureact::context ctx;

        ureact::var_signal<std::vector<std::string>> data
            = make_var( ctx, std::vector<std::string>{} );

        CHECK( data.value() == std::vector<std::string>{} );

        data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "Hello" ); } );

        CHECK( data.value() == std::vector<std::string>{ "Hello" } );

        data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "World!" ); } );

        CHECK( data.value() == std::vector<std::string>{ "Hello", "World!" } );
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

        CHECK( z.value() == 4 );
        CHECK( z_values == std::vector<int>{} );

        a <<= 2; // z is changed to 6
        b <<= 2; // z is changed to 8

        ctx.do_transaction( [&] {
            a <<= 4;
            b <<= 4;
        } ); // z is changed to 16

        CHECK( z_values == std::vector<int>{ 6, 8, 16 } );
    }

    TEST_CASE( "Reacting to value changes" )
    {
        ureact::context ctx;

        ureact::var_signal<int> x = make_var( ctx, 1 );
        ureact::signal<int> xAbs = make_signal( x, []( int value ) { return abs( value ); } );

        std::vector<int> xAbs_values;
        observe( xAbs, [&]( int new_value ) { xAbs_values.push_back( new_value ); } );

        CHECK( xAbs.value() == 1 );
        CHECK( xAbs_values == std::vector<int>{} );

        x <<= 2;  // xAbs is changed to 2
        x <<= -3; // xAbs is changed to 3
        x <<= 3;  // no output, xAbs is still 3

        ctx.do_transaction( [&] {
            x <<= 4;
            x <<= -2;
            x <<= 3;
        } ); // no output, result value of xAbs is still 3

        CHECK( xAbs_values == std::vector<int>{ 2, 3 } );
    }
}
