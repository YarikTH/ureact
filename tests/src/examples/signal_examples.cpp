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

        // make_value is available as a free function and as context's member function
        ureact::value<std::string> firstWord = make_value( ctx, std::string( "Change" ) );
        ureact::value<std::string> secondWord = ctx.make_value( std::string( "me!" ) );

        ureact::function<std::string> bothWords;

        auto concatFunc = []( const std::string& first, const std::string& second ) -> std::string {
            return first + " " + second;
        };

        // Several alternative variants that do exactly the same
        SUBCASE( "using overloaded operators" )
        {
            bothWords = firstWord + " " + secondWord;
        }

        SUBCASE( "using make_function()" )
        {
            bothWords = make_function( with( firstWord, secondWord ), concatFunc );
        }

        SUBCASE( "operators , and |" )
        {
            // operator "," can be used instead of function with()
            // operator "|" can be used instead of make_function()
            bothWords = /*with*/ ( firstWord, secondWord ) | concatFunc;
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

        ureact::value<std::vector<std::string>> data
            = make_value( ctx, std::vector<std::string>{} );

        CHECK( data.get() == std::vector<std::string>{} );

        data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "Hello" ); } );

        CHECK( data.get() == std::vector<std::string>{ "Hello" } );

        data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "World!" ); } );

        CHECK( data.get() == std::vector<std::string>{ "Hello", "World!" } );
    }

    TEST_CASE( "Changing multiple inputs" )
    {
        ureact::context ctx;

        ureact::value<int> a = make_value( ctx, 1 );
        ureact::value<int> b = make_value( ctx, 1 );

        ureact::function<int> x = a + b;
        ureact::function<int> y = a + b;
        ureact::function<int> z = x + y; // 2*(a+b)

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

        ctx.do_transaction( [&] {
            a <<= 4;
            b <<= 4;
        } ); // z is changed to 16

        CHECK( z_values == std::vector<int>{ 6, 8, 16 } );
    }

    TEST_CASE( "Reacting to value changes" )
    {
        ureact::context ctx;

        ureact::value<int> x = make_value( ctx, 1 );
        ureact::function<int> xAbs = make_function( x, []( int value ) { return abs( value ); } );

        std::vector<int> xAbs_values;
        observe( xAbs, [&]( int new_value ) { xAbs_values.push_back( new_value ); } );

        CHECK( xAbs.get() == 1 );
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
