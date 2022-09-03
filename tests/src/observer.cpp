//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// default constructive
static_assert( std::is_default_constructible_v<ureact::observer> );

// nothrow movable only
static_assert( !std::is_copy_constructible_v<ureact::observer> );
static_assert( !std::is_copy_assignable_v<ureact::observer> );
static_assert( std::is_move_constructible_v<ureact::observer> );
static_assert( std::is_move_assignable_v<ureact::observer> );
static_assert( std::is_nothrow_move_constructible_v<ureact::observer> );
static_assert( std::is_nothrow_move_assignable_v<ureact::observer> );


// scoped_observer is not intended to be default constructive, but move constructive from observer
static_assert( !std::is_default_constructible_v<ureact::scoped_observer> );
static_assert( std::is_constructible_v<ureact::scoped_observer, ureact::observer&&> );
static_assert( !std::is_constructible_v<ureact::scoped_observer, const ureact::observer&> );

// nothrow movable only
static_assert( !std::is_copy_constructible_v<ureact::scoped_observer> );
static_assert( !std::is_copy_assignable_v<ureact::scoped_observer> );
static_assert( std::is_move_constructible_v<ureact::scoped_observer> );
static_assert( std::is_move_assignable_v<ureact::scoped_observer> );
static_assert( std::is_nothrow_move_constructible_v<ureact::scoped_observer> );
static_assert( std::is_nothrow_move_assignable_v<ureact::scoped_observer> );

// TODO: test .detach()


// Functor used for observer can optionally return value
// Using this value observer can be optionally self detached
TEST_CASE( "SelfObserverDetach" )
{
    ureact::context ctx;

    std::vector<int> results;
    auto observe_handler = [&]( int v ) {
        if( v < 0 )
        {
            return ureact::observer_action::stop_and_detach;
        }
        else
        {
            results.push_back( v );
            return ureact::observer_action::next;
        }
    };

    std::vector input_values{ 1, 2, 3, -1, 4 };

    SUBCASE( "signal" )
    {
        auto src = ureact::make_var<int>( ctx, -1 );

        ureact::observer obs = observe( src, observe_handler );

        for( int i : input_values )
            src <<= i;
    }
    SUBCASE( "source" )
    {
        auto src = ureact::make_source<int>( ctx );

        ureact::observer obs = observe( src, observe_handler );

        for( int i : input_values )
            src << i;
    }

    CHECK( results == std::vector{ 1, 2, 3 } );
}

// During its lifetime, the observer handle of an observer guarantees that the
// observed subject will not be destroyed and allows explicit detach.
// But even after the observer handle is destroyed, the subject may continue to exist
// and so will the observer.
//
// scoped_observer has similar semantics to a scoped lock.
// When it's destroyed, it detaches and destroys the observer.
TEST_CASE( "ScopedObserver" )
{
    ureact::context ctx;

    auto src = make_source( ctx );

    size_t observation_count = 0;
    auto observe_handler = [&]( auto&& ) { ++observation_count; };

    CHECK( observation_count == 0 );

    SUBCASE( "ScopedObserver" )
    {
        ureact::scoped_observer obs = observe( src, observe_handler );

        src.emit();
    }
    SUBCASE( "ObserverDetach" )
    {
        ureact::observer obs = observe( src, observe_handler );

        src.emit();

        obs.detach(); // scoped_observer does the same in the descriptor
    }

    CHECK( observation_count == 1 );

    src.emit(); // observer is detached, observation is not performed

    CHECK( observation_count == 1 );
}

TEST_CASE( "NoObserveOnNoChanged" )
{
    ureact::context ctx;

    auto a = make_var( ctx, 1 );
    auto b = make_var( ctx, 1 );

    auto product = a * b;

    auto expressionString
        = lift( with( a, b, product ), []( const int a_, const int b_, const int product_ ) {
              return std::to_string( a_ ) + " * " + std::to_string( b_ ) + " = "
                   + std::to_string( product_ );
          } );

    int aObserveCount = 0;
    int bObserveCount = 0;
    int productObserveCount = 0;

    observe( a, [&]( int /*v*/ ) { ++aObserveCount; } );
    observe( b, [&]( int /*v*/ ) { ++bObserveCount; } );
    observe( product, [&]( int /*v*/ ) { ++productObserveCount; } );

    CHECK( aObserveCount == 0 );
    CHECK( bObserveCount == 0 );
    CHECK( productObserveCount == 0 );
    CHECK( expressionString.get() == "1 * 1 = 1" );

    b <<= 2;
    CHECK( aObserveCount == 0 );
    CHECK( bObserveCount == 1 );
    CHECK( productObserveCount == 1 );
    CHECK( expressionString.get() == "1 * 2 = 2" );

    b <<= 2; // Shouldn't change
    CHECK( aObserveCount == 0 );
    CHECK( bObserveCount == 1 );
    CHECK( productObserveCount == 1 );
    CHECK( expressionString.get() == "1 * 2 = 2" );

    do_transaction( ctx, [&]() {
        b <<= 1;
        b <<= 2; // Shouldn't change
    } );
    CHECK( aObserveCount == 0 );
    CHECK( bObserveCount == 1 );
    CHECK( productObserveCount == 1 );
    CHECK( expressionString.get() == "1 * 2 = 2" );

    a <<= 0;
    CHECK( aObserveCount == 1 );
    CHECK( bObserveCount == 1 );
    CHECK( productObserveCount == 2 );
    CHECK( expressionString.get() == "0 * 2 = 0" );

    b <<= 3;
    CHECK( aObserveCount == 1 );
    CHECK( bObserveCount == 2 );
    CHECK( productObserveCount == 2 ); // Product shouldn't change
    CHECK( expressionString.get() == "0 * 3 = 0" );
}

TEST_CASE( "SyncedObserveTest" )
{
    ureact::context ctx;

    auto in1 = make_var( ctx, 1 );
    auto in2 = make_var( ctx, 1 );

    auto sum = lift( with( in1, in2 ), []( int a, int b ) { return a + b; } );
    auto prod = lift( with( in1, in2 ), []( int a, int b ) { return a * b; } );
    auto diff = lift( with( in1, in2 ), []( int a, int b ) { return a - b; } );

    auto src1 = ureact::make_source( ctx );
    auto src2 = ureact::make_source<int>( ctx );

    observe( src1, with( sum, prod, diff ), []( ureact::unit, int sum, int prod, int diff ) {
        CHECK_EQ( sum, 33 );
        CHECK_EQ( prod, 242 );
        CHECK_EQ( diff, 11 );
    } );

    observe( src2, with( sum, prod, diff ), []( int e, int sum, int prod, int diff ) {
        CHECK_EQ( e, 42 );
        CHECK_EQ( sum, 33 );
        CHECK_EQ( prod, 242 );
        CHECK_EQ( diff, 11 );
    } );

    in1 <<= 22;
    in2 <<= 11;

    src1.emit();
    src2.emit( 42 );
}

TEST_CASE( "Observers" )
{
    ureact::context ctx;

    auto x = make_var( ctx, 0 );

    auto identity = []( const int value ) { return value; };

    std::vector<int> x_values;

    auto on_x_value_change = [&]( const int new_value ) { x_values.push_back( new_value ); };

    CHECK( x_values == std::vector<int>{} );

    SUBCASE( "Subject-bound observers v1" )
    {
        // Inner scope
        {
            // Create a signal in the function scope
            auto my_signal = lift( x, identity );

            // The lifetime of the observer is bound to my_signal.
            // After scope my_signal is destroyed, and so is the observer
            observe( my_signal, on_x_value_change );

            x <<= 1; // output: 2

            CHECK( x_values == std::vector<int>{ 1 } );
        }
        // ~Inner scope

        x <<= 2; // no output

        CHECK( x_values == std::vector<int>{ 1 } );
    }

    SUBCASE( "Subject-bound observers v2" )
    {
        // Outer scope
        {
            // Unbound observer
            ureact::observer obs;

            // Inner scope
            {
                auto my_signal = lift( x, identity );

                // Move-assign to obs
                obs = observe( my_signal, on_x_value_change );

                // The node linked to my_signal is now also owned by obs

                x <<= 1; // output: 2

                CHECK( x_values == std::vector<int>{ 1 } );
            }
            // ~Inner scope

            // my_signal was destroyed, but as long as obs exists and is still
            // attached to the signal node, this signal node won't be destroyed

            x <<= 2; // output: 3

            CHECK( x_values == std::vector<int>{ 1, 2 } );
        }
        // ~Outer scope

        // obs was destroyed
        // -> the signal node is no longer owned by anything and is destroyed
        // -> the observer node is destroyed as it was bound to the subject

        x <<= 3; // no output

        CHECK( x_values == std::vector<int>{ 1, 2 } );
    }
}