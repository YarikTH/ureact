//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/signal.hpp"

#include "catch2_extra.hpp"
#include "identity.hpp"
#include "ureact/adaptor/lift.hpp"

// copyable and nothrow movable
static_assert( std::is_default_constructible_v<ureact::signal<int>> );
static_assert( std::is_copy_constructible_v<ureact::signal<int>> );
static_assert( std::is_copy_assignable_v<ureact::signal<int>> );
static_assert( std::is_move_constructible_v<ureact::signal<int>> );
static_assert( std::is_move_assignable_v<ureact::signal<int>> );
static_assert( std::is_nothrow_move_constructible_v<ureact::signal<int>> );
static_assert( std::is_nothrow_move_assignable_v<ureact::signal<int>> );

// copyable and nothrow movable
static_assert( std::is_default_constructible_v<ureact::var_signal<int>> );
static_assert( std::is_copy_constructible_v<ureact::var_signal<int>> );
static_assert( std::is_copy_assignable_v<ureact::var_signal<int>> );
static_assert( std::is_move_constructible_v<ureact::var_signal<int>> );
static_assert( std::is_move_assignable_v<ureact::var_signal<int>> );
static_assert( std::is_nothrow_move_constructible_v<ureact::var_signal<int>> );
static_assert( std::is_nothrow_move_assignable_v<ureact::var_signal<int>> );

// TODO: check type traits for signals
// * temp_signal<S, ...>
// * signal_pack<S...>

TEST_CASE( "ureact::signal<S> (construction)" )
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

TEST_CASE( "ureact::signal<S> (assignment construction)" )
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

TEST_CASE( "ureact::var_signal<S> (construction)" )
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

TEST_CASE( "ureact::var_signal<S> (assignment construction)" )
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

TEST_CASE( "ureact::signal<S> (get)" )
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

TEST_CASE( "ureact::signal<S> (arrow access)" )
{
    ureact::context ctx;

    ureact::var_signal src = ureact::make_var( ctx, std::make_pair( 1, 2 ) );
    const ureact::signal sig = ureact::lift( src, identity{} );

    // constant access to member values and methods using -> operator
    // It is a short form of sig.get().<member> / sig().<member>
    CHECK( sig->first == 1 );
    CHECK( sig->second == 2 );

    src.set( std::make_pair( 4, 2 ) );

    CHECK( sig.get().first == 4 );
    CHECK( sig.get().second == 2 );
}

TEST_CASE( "ureact::var_signal<S> (set)" )
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

TEST_CASE( "ureact::var_signal<S> (modify)" )
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
