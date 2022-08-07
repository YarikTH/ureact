//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// clang-format off
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>
// clang-format on

#include <numeric>

#include "doctest_extra.h"
#include "copy_stats.hpp"
#include "ureact/ureact.hpp"

// type_traits to test:
// * context
// * event_emitter
// * event_range
// * events<E>
// * event_source<E>
// * event_source<E>::iterator
// * signal<S>
// * var_signal<S>
// * temp_signal<S, ...>
// * signal_pack
// * observer
// * scoped_observer

// * detail::node_base
// * detail::reactive_base

// copyable and nothrow movable
static_assert( std::is_default_constructible_v<ureact::events<>> );
static_assert( std::is_copy_constructible_v<ureact::events<>> );
static_assert( std::is_copy_assignable_v<ureact::events<>> );
static_assert( std::is_move_constructible_v<ureact::events<>> );
static_assert( std::is_move_assignable_v<ureact::events<>> );
static_assert( std::is_nothrow_move_constructible_v<ureact::events<>> );
static_assert( std::is_nothrow_move_assignable_v<ureact::events<>> );


// default constructive
static_assert( std::is_default_constructible_v<ureact::event_source<>> );

// constructive from context
static_assert( std::is_constructible_v<ureact::event_source<>, ureact::context&> );

// copyable and nothrow movable
static_assert( std::is_copy_constructible_v<ureact::event_source<>> );
static_assert( std::is_copy_assignable_v<ureact::event_source<>> );
static_assert( std::is_move_constructible_v<ureact::event_source<>> );
static_assert( std::is_move_assignable_v<ureact::event_source<>> );
static_assert( std::is_nothrow_move_constructible_v<ureact::event_source<>> );
static_assert( std::is_nothrow_move_assignable_v<ureact::event_source<>> );


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


TEST_CASE( "CopyStatsForSignalCalculations" )
{
    ureact::context ctx;

    copy_stats stats;

    auto a = ureact::make_var( ctx, copy_counter{ 1, &stats } );
    auto b = ureact::make_var( ctx, copy_counter{ 10, &stats } );
    auto c = ureact::make_var( ctx, copy_counter{ 100, &stats } );
    auto d = ureact::make_var( ctx, copy_counter{ 1000, &stats } );

    // 4x move to m_value_
    // 4x copy to m_new_value (can't be uninitialized for references)
    CHECK( stats.copy_count == 4 );
    CHECK( stats.move_count == 4 );

    auto x = a + b + c + d;

    CHECK( stats.copy_count == 4 );
    CHECK( stats.move_count == 7 );
    CHECK( x.get().v == 1111 );

    a <<= copy_counter{ 2, &stats };

    CHECK( stats.copy_count == 4 );
    CHECK( stats.move_count == 10 );
    CHECK( x.get().v == 1112 );
}

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

TEST_CASE( "EventsConstruction" )
{
    ureact::context ctx;

    // default constructed events isn't linked to a reactive node, thus
    // can't be used for anything but for following assignment
    SUBCASE( "default constructed" )
    {
        ureact::events<> null_evt;
        CHECK_FALSE( null_evt.is_valid() );
    }

    // events can be created via object slicing from event_source object
    SUBCASE( "slicing" )
    {
        auto src = ureact::make_source<>( ctx );
        ureact::events<> evt = src;

        CHECK( evt.is_valid() );
    }

    // events can be created via make_never call
    SUBCASE( "make_never" )
    {
        ureact::events evt = ureact::make_never<>( ctx );

        CHECK( evt.is_valid() );
    }

    // events can be created using various algorithms
    SUBCASE( "algorithm" )
    {
        auto src1 = ureact::make_source<>( ctx );
        auto src2 = ureact::make_source<>( ctx );
        ureact::events<> evt = merge( src1, src2 );

        CHECK( evt.is_valid() );
    }

    // copy and move construction of events
    SUBCASE( "copy and move constructed" )
    {
        ureact::events<> evt = ureact::make_source<>( ctx );
        CHECK( evt.is_valid() );

        SUBCASE( "copy constructed" )
        {
            ureact::events<> evt_copy = evt;
            CHECK( evt_copy.is_valid() );
            CHECK( evt.is_valid() );
        }

        SUBCASE( "move constructed" )
        {
            ureact::events<> evt_move = std::move( evt );
            CHECK( evt_move.is_valid() );
            CHECK_FALSE( evt.is_valid() );
        }
    }
}

TEST_CASE( "EventsAssignmentConstruction" )
{
    ureact::context ctx;

    ureact::events<> src = ureact::make_source<>( ctx );
    CHECK( src.is_valid() );

    SUBCASE( "copy assignment" )
    {
        ureact::events<> src_copy;
        CHECK_FALSE( src_copy.is_valid() );

        src_copy = src;
        CHECK( src_copy.is_valid() );
        CHECK( src.is_valid() );
        CHECK( src_copy.equals( src ) );
    }

    SUBCASE( "move assignment" )
    {
        ureact::events<> src_move;
        CHECK_FALSE( src_move.is_valid() );

        src_move = std::move( src );
        CHECK( src_move.is_valid() );
        CHECK_FALSE( src.is_valid() );
    }
}

TEST_CASE( "EventSourceConstruction" )
{
    ureact::context ctx;

    // default constructed event_source isn't linked to a reactive node, thus
    // can't be used for anything but for following assignment
    SUBCASE( "default constructed" )
    {
        ureact::event_source<> null_src;
        CHECK_FALSE( null_src.is_valid() );
    }

    // event_source can be created via free function semantically close to std::make_shared
    // Event value type E has to be specified explicitly. It would be token if it is omitted
    SUBCASE( "make_source<>()" )
    {
        auto src = ureact::make_source<>( ctx );
        CHECK( src.is_valid() );
    }

    // event_source can be created using constructor receiving context reference
    SUBCASE( "fully constructed" )
    {
        ureact::event_source<int> src{ ctx };
        CHECK( src.is_valid() );
    }

    // event_source can be created using constructor receiving context reference
    // in the form of AAA
    SUBCASE( "fully constructed AAA" )
    {
        auto src = ureact::event_source<int>{ ctx };
        CHECK( src.is_valid() );
    }

    // copy and move construction of event_source
    SUBCASE( "copy and move constructed" )
    {
        ureact::event_source<int> src{ ctx };
        CHECK( src.is_valid() );

        SUBCASE( "copy constructed" )
        {
            ureact::event_source<int> src_copy = src;
            CHECK( src_copy.is_valid() );
            CHECK( src.is_valid() );
            CHECK( src_copy.equals( src ) );
        }

        SUBCASE( "move constructed" )
        {
            ureact::event_source<int> src_move = std::move( src );
            CHECK( src_move.is_valid() );
            CHECK_FALSE( src.is_valid() );
        }
    }
}

TEST_CASE( "EventSourceAssignmentConstruction" )
{
    ureact::context ctx;

    ureact::event_source<int> src{ ctx };
    CHECK( src.is_valid() );

    SUBCASE( "copy assignment" )
    {
        ureact::event_source<int> src_copy;
        CHECK_FALSE( src_copy.is_valid() );

        src_copy = src;
        CHECK( src_copy.is_valid() );
        CHECK( src.is_valid() );
        CHECK( src_copy.equals( src ) );
    }

    SUBCASE( "move assignment" )
    {
        ureact::event_source<int> src_move;
        CHECK_FALSE( src_move.is_valid() );

        src_move = std::move( src );
        CHECK( src_move.is_valid() );
        CHECK_FALSE( src.is_valid() );
    }
}

// Events has shared_ptr semantics. They are literally shared_ptr to reactive node
// that does all the work and form dependency tree
TEST_CASE( "EventsSmartPointerSemantics" )
{
    ureact::context ctx;

    ureact::event_source<int> src{ ctx };

    const auto is_even = []( auto i ) { return i % 2 == 0; };
    const auto is_odd = []( auto i ) { return i % 2 == 1; };

    auto filtered = src | ureact::filter( is_even );

    auto result_even = ureact::collect<std::vector>( filtered );

    //      src         //
    //       |          //
    // filter(is_even)  //
    //       |          //
    //  result_even     //

    // reassigning of 'filtered' doesn't affect result_even, because it depends not on
    // 'filtered' itself, but on reactive node it pointed before
    filtered = src | ureact::filter( is_odd );

    auto result_odd = ureact::collect<std::vector>( filtered );

    //               src                //
    //            /       \             //
    // filter(is_even)   filter(is_odd) //
    //       |                 |        //
    //  result_even        result_odd   //

    std::vector<int> v{ 1, 2, 3, 4, 5, 6 };
    std::copy( v.begin(), v.end(), src.begin() );

    CHECK_EQ( result_even.get(), std::vector{ 2, 4, 6 } );
    CHECK_EQ( result_odd.get(), std::vector{ 1, 3, 5 } );
}

// We can emit events using a bunch of methods doing basically the same
TEST_CASE( "EventSourceEmitting" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto _2 = 2;

    auto result = ureact::collect<std::vector>( src );

    SUBCASE( "emit method" )
    {
        src.emit( 1 );  // R-value
        src.emit( _2 ); // L-value
    }
    SUBCASE( "function object" )
    {
        src( 1 );  // R-value
        src( _2 ); // L-value
    }
    SUBCASE( "stream" )
    {
        src << 1   // R-value
            << _2; // L-value
    }
    SUBCASE( "stl iterator" )
    {
        std::generate_n( src.begin(), 1, [] { return 1; } );                   // R-value
        std::generate_n( src.begin(), 1, [&]() -> const int& { return _2; } ); // L-value
    }

    CHECK_EQ( result.get(), std::vector{ 1, 2 } );
}

// We can emit tokens using a bunch of methods doing basically the same
TEST_CASE( "EventSourceEmittingTokenSpecialization" )
{
    ureact::context ctx;

    auto src = ureact::make_source<>( ctx );
    auto unit = ureact::unit{};

    auto counted = src | ureact::count();

    SUBCASE( "emit method" )
    {
        src.emit();                 // event_source<token> specialization without argument
        src.emit( ureact::unit{} ); // R-value
        src.emit( unit );           // L-value
    }
    SUBCASE( "function object" )
    {
        src();                 // event_source<token> specialization without argument
        src( ureact::unit{} ); // R-value
        src( unit );           // L-value
    }
    SUBCASE( "stream" )
    {
        src << ureact::unit{} // R-value
            << unit           // L-value
            << unit;          // L-value
    }
    SUBCASE( "stl iterator" )
    {
        std::generate_n( src.begin(), 1, [] { return ureact::unit{}; } );           // R-value
        std::generate_n( src.begin(), 2, [&]() -> ureact::unit& { return unit; } ); // L-value
    }

    CHECK( counted.get() == 3 );
}
