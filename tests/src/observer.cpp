//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "doctest_extra.h"
#include "identity.hpp"
#include "ureact/events.hpp"
#include "ureact/lift.hpp"
#include "ureact/observe.hpp"
#include "ureact/transaction.hpp"

// default constructive
static_assert( std::is_default_constructible_v<ureact::observer> );

// nothrow movable only
static_assert( !std::is_copy_constructible_v<ureact::observer> );
static_assert( !std::is_copy_assignable_v<ureact::observer> );
static_assert( std::is_move_constructible_v<ureact::observer> );
static_assert( std::is_move_assignable_v<ureact::observer> );
static_assert( std::is_nothrow_move_constructible_v<ureact::observer> );
static_assert( std::is_nothrow_move_assignable_v<ureact::observer> );


// TODO: consider separation of observer.cpp into observer.cpp and observe.cpp
// TODO: check construction and moving
// TODO: test .detach() .is_valid()
// TODO: test observe with temporary events. Synced and not
// TODO: order tests (less specific to more specific)
// TODO: refactor NoObserveOnNoChanged

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

    auto a = make_var( ctx, 1 );
    auto b = make_var( ctx, 1 );

    const auto sum = a + b;
    const auto prod = a * b;
    const auto diff = a - b;

    SUBCASE( "unit source" )
    {
        const auto src = ureact::make_source( ctx );
        using result_t = std::vector<std::tuple<int, int, int>>;

        result_t result;

        observe( src, with( sum, prod, diff ), [&]( ureact::unit, int sum, int prod, int diff ) {
            result.emplace_back( sum, prod, diff );
        } );

        a <<= 22;
        b <<= 11;

        CHECK( result.empty() );

        src.emit();

        const result_t expected = { { 33, 242, 11 } };

        CHECK( result == expected );
    }

    SUBCASE( "string source" )
    {
        const auto src = ureact::make_source<std::string>( ctx );
        using result_t = std::vector<std::tuple<std::string, int, int, int>>;

        result_t result;

        observe(
            src, with( sum, prod, diff ), [&]( const std::string& e, int sum, int prod, int diff ) {
                result.emplace_back( e, sum, prod, diff );
            } );

        a <<= 22;
        b <<= 11;

        CHECK( result.empty() );

        src.emit( "Wtf?" );

        const result_t expected = { { "Wtf?", 33, 242, 11 } };

        CHECK( result == expected );
    }
}

TEST_CASE( "Observers" )
{
    ureact::context ctx;

    auto x = make_var( ctx, 0 );

    std::vector<int> x_values;

    auto on_x_value_change = [&]( const int new_value ) { x_values.push_back( new_value ); };

    CHECK( x_values == std::vector<int>{} );

    SUBCASE( "Subject-bound observers v1" )
    {
        // Inner scope
        {
            // Create a signal in the function scope
            auto my_signal = lift( x, identity{} );

            // The lifetime of the observer is bound to my_signal.
            // After scope my_signal is destroyed, and so is the observer
            observe( my_signal, on_x_value_change );

            x <<= 1;

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
                auto my_signal = lift( x, identity{} );

                // Move-assign to obs
                obs = observe( my_signal, on_x_value_change );

                // The node linked to my_signal is now also owned by obs

                x <<= 1;

                CHECK( x_values == std::vector<int>{ 1 } );
            }
            // ~Inner scope

            // my_signal was destroyed, but as long as obs exists and is still
            // attached to the signal node, this signal node won't be destroyed

            x <<= 2;

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
