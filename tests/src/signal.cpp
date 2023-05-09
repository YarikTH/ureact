//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "catch2_extra.hpp"
#include "ureact/adaptor/lift.hpp"

// TODO: check type traits for signals
// * signal<S>
// * var_signal<S>
// * temp_signal<S, ...>
// * signal_pack

TEST_CASE( "SignalConstruction" )
{
    ureact::context ctx;

    // default constructed signal isn't linked to a reactive node, thus
    // can't be used for anything but for following assignment
    SECTION( "default constructed" )
    {
        ureact::signal<int> null_signal;
        CHECK_FALSE( null_signal.is_valid() );
    }

    // signal can be created via object slicing from var_signal object
    SECTION( "slicing" )
    {
        auto var = ureact::make_var<int>( ctx, 5 );
        ureact::signal sig = var;

        CHECK( sig.is_valid() );
    }

    // signal can be created via make_const call
    SECTION( "make_const" )
    {
        ureact::signal sig = ureact::make_const<int>( ctx, 5 );

        CHECK( sig.is_valid() );
    }

    // signal can be created using various algorithms (in particular using overloaded operator)
    SECTION( "algorithm" )
    {
        auto var = ureact::make_var<int>( ctx, 5 );
        ureact::signal sig = ureact::lift( var, []( int a ) { return a; } );

        CHECK( sig.is_valid() );
    }

    // copy and move construction of signal
    SECTION( "copy and move constructed" )
    {
        ureact::signal<int> sig = ureact::make_var<int>( ctx, 42 );
        CHECK( sig.is_valid() );

        SECTION( "copy constructed" )
        {
            ureact::signal sig_copy = sig;
            CHECK( sig_copy.is_valid() );
            CHECK( sig.is_valid() );
        }

        SECTION( "move constructed" )
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

    SECTION( "copy assignment" )
    {
        ureact::signal<int> src_copy;
        CHECK_FALSE( src_copy.is_valid() );

        src_copy = src;
        CHECK( src_copy.is_valid() );
        CHECK( src.is_valid() );
        CHECK( src_copy.equal_to( src ) );
    }

    SECTION( "move assignment" )
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
    SECTION( "default constructed" )
    {
        ureact::var_signal<int> null_var;
        CHECK_FALSE( null_var.is_valid() );
    }

    // var_signal can be created via free function semantically close to std::make_shared
    // Signal value type S can be specified explicitly, but doesn't have to.
    SECTION( "make_var()" )
    {
        auto var = ureact::make_var( ctx, 8 );
        CHECK( var.is_valid() );
    }
    SECTION( "make_var<T>()" )
    {
        auto var = ureact::make_var<float>( ctx, 6 );
        CHECK( var.is_valid() );
    }

    // copy and move construction of var_signal
    SECTION( "copy and move constructed" )
    {
        ureact::var_signal src = ureact::make_var( ctx, 0 );
        CHECK( src.is_valid() );

        SECTION( "copy constructed" )
        {
            ureact::var_signal<int> src_copy = src;
            CHECK( src_copy.is_valid() );
            CHECK( src.is_valid() );
            CHECK( src_copy.equal_to( src ) );
        }

        SECTION( "move constructed" )
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

    ureact::var_signal src = ureact::make_var( ctx, -1 );
    CHECK( src.is_valid() );

    SECTION( "copy assignment" )
    {
        ureact::var_signal<int> src_copy;
        CHECK_FALSE( src_copy.is_valid() );

        src_copy = src;
        CHECK( src_copy.is_valid() );
        CHECK( src.is_valid() );
        CHECK( src_copy.equal_to( src ) );
    }

    SECTION( "move assignment" )
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

    ureact::var_signal x = ureact::make_var( ctx, 1 );

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

    ureact::var_signal src = ureact::make_var( ctx, 1 );
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

    ureact::var_signal src = ureact::make_var( ctx, 1 );

    CHECK( src.get() == 1 );

    int new_value{ 5 };

    SECTION( "set method L-value" )
    {
        src.set( new_value );
    }
    SECTION( "set method R-value" )
    {
        src.set( 5 );
    }
    SECTION( "set operator L-value" )
    {
        src <<= new_value;
    }
    SECTION( "set operator R-value" )
    {
        src <<= 5;
    }
    SECTION( "set function object L-value" )
    {
        src( new_value );
    }
    SECTION( "set function object R-value" )
    {
        src( 5 );
    }

    CHECK( src.get() == 5 );
}

TEST_CASE( "VarSignalModifyValue" )
{
    ureact::context ctx;

    ureact::var_signal src = ureact::make_var( ctx, 9 );

    CHECK( src.get() == 9 );

    auto change_int_to_5 = []( int& v ) { v = 5; };

    SECTION( "modify method" )
    {
        src.modify( change_int_to_5 );
    }
    SECTION( "modify operator" )
    {
        src <<= change_int_to_5;
    }

    CHECK( src.get() == 5 );
}
