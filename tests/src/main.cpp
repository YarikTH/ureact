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

#include "tests_stdafx.hpp"
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

    // signal can be created using various algorithms (in particular using overloaded operator)
    SUBCASE( "algorithm" )
    {
        auto var = ureact::make_var<int>( ctx, 5 );
        ureact::signal sig = make_signal( var, []( int a ) { return a; } );

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
    auto token = ureact::token::value;

    auto counted = src | ureact::count();

    SUBCASE( "emit method" )
    {
        src.emit();                       // event_source<token> specialization without argument
        src.emit( ureact::token::value ); // R-value
        src.emit( token );                // L-value
    }
    SUBCASE( "function object" )
    {
        src();                       // event_source<token> specialization without argument
        src( ureact::token::value ); // R-value
        src( token );                // L-value
    }
    SUBCASE( "stream" )
    {
        src << ureact::token::value // R-value
            << token                // L-value
            << token;               // L-value
    }
    SUBCASE( "stl iterator" )
    {
        std::generate_n( src.begin(), 1, [] { return ureact::token::value; } );       // R-value
        std::generate_n( src.begin(), 2, [&]() -> ureact::token& { return token; } ); // L-value
    }

    CHECK( counted.get() == 3 );
}

// calculate sum and product of range of integers using value based function
TEST_CASE( "FoldByValue" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::signal<int> sum;
    ureact::signal<int> product;
    int plus_calls = 0;
    int multiplies_calls = 0;

    auto plus = [&]( int accum, int value ) {
        ++plus_calls;
        return accum + value;
    };
    auto batch_multiplies = [&]( int accum, ureact::event_range<int> values ) {
        ++multiplies_calls;
        return std::accumulate( values.begin(), values.end(), accum, std::multiplies<>() );
    };

    SUBCASE( "Functional syntax" )
    {
        sum = ureact::fold( src, 0, plus );
        product = ureact::fold( src, 1, batch_multiplies );
    }
    SUBCASE( "Piped syntax" )
    {
        sum = src | ureact::fold( 0, plus );
        product = src | ureact::fold( 1, batch_multiplies );
    }

    std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // do a single transaction to show the difference
    do_transaction( ctx, [&]() { //
        std::copy( v.begin(), v.end(), src.begin() );
    } );

    CHECK( sum.get() == 55 );
    CHECK( product.get() == 3628800 );
    CHECK( plus_calls == 10 );
    CHECK( multiplies_calls == 1 );
}

// calculate sum and product of range of integers using reference based function
TEST_CASE( "FoldByRef" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::signal<int> sum_byval;
    ureact::signal<int> sum_byref;
    ureact::signal<int> product_byref;
    int plus_calls = 0;
    int multiplies_calls = 0;

    auto plus_ref = [&]( int& accum, int value ) {
        ++plus_calls;
        accum += value;
    };
    auto batch_multiplies_ref = [&]( int& accum, ureact::event_range<int> values ) {
        ++multiplies_calls;
        accum = std::accumulate( values.begin(), values.end(), accum, std::multiplies<>() );
    };

    SUBCASE( "Functional syntax" )
    {
        sum_byval = ureact::fold( src, 0, std::plus<>() );
        sum_byref = ureact::fold( src, 0, plus_ref );
        product_byref = ureact::fold( src, 1, batch_multiplies_ref );
    }
    SUBCASE( "Piped syntax" )
    {
        sum_byval = src | ureact::fold( 0, std::plus<>() );
        sum_byref = src | ureact::fold( 0, plus_ref );
        product_byref = src | ureact::fold( 1, batch_multiplies_ref );
    }

    auto sum_byval_changes = count( monitor( sum_byval ) );
    auto sum_byref_changes = count( monitor( sum_byref ) );

    std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // do a single transaction to show the difference
    do_transaction( ctx, [&]() { //
        std::copy( v.begin(), v.end(), src.begin() );
    } );

    CHECK( sum_byval.get() == 55 );
    CHECK( sum_byref.get() == 55 );
    CHECK( product_byref.get() == 3628800 );

    // value actually changed once
    CHECK( sum_byval_changes.get() == 1 );
    CHECK( sum_byref_changes.get() == 1 );

    // batch processing at work
    CHECK( plus_calls == 10 );
    CHECK( multiplies_calls == 1 );

    src.emit( 0 );
    src.emit( 0 );
    src.emit( 0 );

    CHECK( sum_byval.get() == 55 );
    CHECK( sum_byref.get() == 55 );
    CHECK( product_byref.get() == 0 );

    // passing 0 don't change the sum, but referenced fold reports change anyway
    CHECK( sum_byval_changes.get() == 1 );
    CHECK( sum_byref_changes.get() == 4 );

    // plus 3 calls each
    CHECK( plus_calls == 13 );
    CHECK( multiplies_calls == 4 );
}

// calculate sum of range of integers multiplied by using value based function
TEST_CASE( "FoldByValueSynced" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto mult = ureact::make_var( ctx, 1 );
    ureact::signal<int> sum;
    ureact::signal<int> batch_sum;
    int plus_calls = 0;
    int batch_plus_calls = 0;

    auto plus = [&]( int accum, int value, int mult ) {
        ++plus_calls;
        return accum + value * mult;
    };
    auto batch_plus = [&]( int accum, ureact::event_range<int> values, int mult ) {
        ++batch_plus_calls;
        return accum + std::accumulate( values.begin(), values.end(), 0, std::plus<>() ) * mult;
    };

    SUBCASE( "Functional syntax" )
    {
        sum = ureact::fold( src, 0, with( mult ), plus );
        batch_sum = ureact::fold( src, 0, with( mult ), batch_plus );
    }
    // todo: Piped syntax is not yet supported for synced version
    //    SUBCASE( "Piped syntax" )
    //    {
    //        sum = src | ureact::fold( 0, with(mult), plus );
    //        batch_sum = src | ureact::fold( 1, with(mult), batch_plus );
    //    }

    std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // do a single transaction to show the difference
    do_transaction( ctx, [&]() { //
        std::copy( v.begin(), v.end(), src.begin() );
    } );

    CHECK( sum.get() == 55 );
    CHECK( batch_sum.get() == 55 );
    CHECK( plus_calls == 10 );
    CHECK( batch_plus_calls == 1 );

    do_transaction( ctx, [&]() { //
        mult <<= 0;
        std::copy( v.begin(), v.end(), src.begin() );
    } );

    CHECK( sum.get() == 55 );
    CHECK( batch_sum.get() == 55 );
    CHECK( plus_calls == 20 );
    CHECK( batch_plus_calls == 2 );

    do_transaction( ctx, [&]() { //
        mult <<= 3;
        src << 5;
    } );

    CHECK( sum.get() == 70 );
    CHECK( batch_sum.get() == 70 );
    CHECK( plus_calls == 21 );
    CHECK( batch_plus_calls == 3 );
}

// calculate sum of range of integers multiplied by using reference based function
TEST_CASE( "FoldByRefSynced" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto mult = ureact::make_var( ctx, 1 );
    ureact::signal<int> sum_byval;
    ureact::signal<int> sum_byref;
    ureact::signal<int> batch_sum_byref;
    int plus_calls = 0;
    int batch_plus_calls = 0;

    auto plus_val = []( int accum, int value, int mult ) { return accum + value * mult; };
    auto plus_ref = [&]( int& accum, int value, int mult ) {
        ++plus_calls;
        accum += value * mult;
    };
    auto batch_plus_ref = [&]( int& accum, ureact::event_range<int> values, int mult ) {
        ++batch_plus_calls;
        accum += std::accumulate( values.begin(), values.end(), accum, std::plus<>() ) * mult;
    };

    SUBCASE( "Functional syntax" )
    {
        sum_byval = ureact::fold( src, 0, with( mult ), plus_val );
        sum_byref = ureact::fold( src, 0, with( mult ), plus_ref );
        batch_sum_byref = ureact::fold( src, 0, with( mult ), batch_plus_ref );
    }
    // todo: Piped syntax is not yet supported for synced version
    //    SUBCASE( "Piped syntax" )
    //    {
    //        sum_byval = src | ureact::fold( 0, with( mult ), plus_val );
    //        sum_byref = src | ureact::fold( 0, with( mult ), plus_ref );
    //        batch_sum_byref = src | ureact::fold( 0, with( mult ), batch_plus_ref );
    //    }

    auto sum_byval_changes = count( monitor( sum_byval ) );
    auto sum_byref_changes = count( monitor( sum_byref ) );

    std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // do a single transaction to show the difference
    do_transaction( ctx, [&]() { //
        std::copy( v.begin(), v.end(), src.begin() );
    } );

    CHECK( sum_byval.get() == 55 );
    CHECK( sum_byref.get() == 55 );
    CHECK( batch_sum_byref.get() == 55 );

    // value actually changed once
    CHECK( sum_byval_changes.get() == 1 );
    CHECK( sum_byref_changes.get() == 1 );

    // batch processing at work
    CHECK( plus_calls == 10 );
    CHECK( batch_plus_calls == 1 );

    mult <<= 0;
    src.emit( 1 );
    src.emit( 2 );
    src.emit( 3 );

    CHECK( sum_byval.get() == 55 );
    CHECK( sum_byref.get() == 55 );
    CHECK( batch_sum_byref.get() == 55 );

    // passing value multiplied by 0 don't change the sum, but referenced fold reports change anyway
    CHECK( sum_byval_changes.get() == 1 );
    CHECK( sum_byref_changes.get() == 4 );

    // plus 3 calls each
    CHECK( plus_calls == 13 );
    CHECK( batch_plus_calls == 4 );
}

// fold integers to receive sum, product and dash concatenated string
// based on example https://en.cppreference.com/w/cpp/algorithm/accumulate
TEST_CASE( "FoldVsAccumulate" )
{
    std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    const auto dash_fold =               //
        []( std::string accum, int i ) { //
            return std::move( accum ) + '-' + std::to_string( i );
        };

    int sum;
    int product;
    std::string dashed;

    SUBCASE( "std::accumulate for example" )
    {
        sum = std::accumulate( v.begin(), v.end(), 0, std::plus<>() );

        product = std::accumulate( v.begin(), v.end(), 1, std::multiplies<>() );

        dashed = std::accumulate( std::next( v.begin(), 1 ),
            v.end(),
            std::to_string( v[0] ), // start with first element
            dash_fold );
    }
    SUBCASE( "ureact::fold" )
    {
        ureact::context ctx;

        auto src = ureact::make_source<int>( ctx );
        ureact::signal<int> sum_s;
        ureact::signal<int> product_s;
        ureact::signal<std::string> dashed_s;

        SUBCASE( "Functional syntax" )
        {
            sum_s = ureact::fold( src, 0, std::plus<>() );
            product_s = ureact::fold( src, 1, std::multiplies<>() );
            dashed_s = ureact::fold( ureact::drop( src, 1 ), //
                std::to_string( v[0] ),
                dash_fold );
        }
        SUBCASE( "Piped syntax" )
        {
            sum_s = src | ureact::fold( 0, std::plus<>() );
            product_s = src | ureact::fold( 1, std::multiplies<>() );
            dashed_s = src | ureact::drop( 1 ) | ureact::fold( std::to_string( v[0] ), dash_fold );
        }

        std::copy( v.begin(), v.end(), src.begin() );

        sum = sum_s.get();
        product = product_s.get();
        dashed = dashed_s.get();
    }

    CHECK( sum == 55 );
    CHECK( product == 3628800 );
    CHECK( dashed == "1-2-3-4-5-6-7-8-9-10" );
}

// zip 3 event sources into 1
// based on example https://en.cppreference.com/w/cpp/ranges/zip_view
TEST_CASE( "Zip" )
{
    ureact::context ctx;

    auto x = ureact::make_source<int>( ctx );
    auto y = ureact::make_source<std::string>( ctx );
    auto z = ureact::make_source<char>( ctx );

    using zipped_t = std::tuple<int, std::string, char>;

    ureact::events<zipped_t> src = zip( x, y, z );

    const auto result = ureact::collect<std::vector>( src );

    // clang-format off
    x <<  1  <<  2  <<  3  <<  4;
    y << "α" << "β" << "γ" << "δ" << "ε";
    z << 'A' << 'B' << 'C' << 'D' << 'E' << 'F';
    // clang-format on

    const std::vector expected = {
        zipped_t{ 1, "α", 'A' },
        zipped_t{ 2, "β", 'B' },
        zipped_t{ 3, "γ", 'C' },
        zipped_t{ 4, "δ", 'D' } //
    };
    CHECK( result.get() == expected );
}

// merge 3 event sources into 1
TEST_CASE( "Merge" )
{
    ureact::context ctx;

    auto src1 = ureact::make_source<int>( ctx );
    auto src2 = ureact::make_source<int>( ctx );
    auto src3 = ureact::make_source<int>( ctx );

    ureact::events<int> src = merge( src1, src2, src3 );

    const auto result = ureact::collect<std::vector>( src );

    src1 << 1 << 5;
    src2 << -1;
    src3 << 9;
    src2 << 0;

    const std::vector<int> expected = { 1, 5, -1, 9, 0 };
    CHECK( result.get() == expected );
}

// on every std::pair<N, value> pass value N times
TEST_CASE( "Process" )
{
    ureact::context ctx;

    auto src = ureact::make_source<std::pair<unsigned, int>>( ctx );
    ureact::events<int> processed;
    int calls = 0;

    const auto repeater = [&calls]( ureact::event_range<std::pair<unsigned, int>> range,
                              ureact::event_emitter<int> out ) {
        for( const auto& [n, value] : range )
            for( unsigned i = 0; i < n; ++i )
                out.emit( value );
        ++calls;
    };

    SUBCASE( "Functional syntax" )
    {
        processed = ureact::process<int>( src, repeater );
    }
    SUBCASE( "Piped syntax" )
    {
        processed = src | ureact::process<int>( repeater );
    }

    const auto result = ureact::collect<std::vector>( processed );

    src.emit( { 2u, -1 } );
    src.emit( { 0u, 666 } );

    do_transaction( ctx, [&]() { src << std::make_pair( 1u, 2 ) << std::make_pair( 3u, 7 ); } );

    const std::vector<int> expected = { -1, -1, 2, 7, 7, 7 };
    CHECK( result.get() == expected );

    CHECK( calls == 3 );
}

// on every value read additional values N and timestamp and pass pair<timestamp, value> N times
TEST_CASE( "ProcessSynced" )
{
    ureact::context ctx;

    using record_t = std::pair<std::string, int>;

    auto src = ureact::make_source<int>( ctx );
    auto n = ureact::make_var<unsigned>( ctx, {} );
    auto timestamp = ureact::make_var<std::string>( ctx, {} );
    ureact::events<record_t> processed;

    const auto repeater = []( ureact::event_range<int> range,
                              ureact::event_emitter<record_t> out,
                              unsigned n,
                              const std::string& timestamp ) {
        for( const auto& value : range )
            for( unsigned i = 0; i < n; ++i )
                out << record_t{ timestamp, value };
    };

    SUBCASE( "Functional syntax" )
    {
        processed = ureact::process<record_t>( src, with( n, timestamp ), repeater );
    }
    // todo: Piped syntax is not yet supported for synced version
    //    SUBCASE( "Piped syntax" )
    //    {
    //        processed = src | ureact::process<record_t>( with( n, timestamp ), repeater );
    //    }

    const auto result = ureact::collect<std::vector>( processed );

    n <<= 2;
    timestamp <<= "1 Jan 2020";
    src.emit( -1 );

    n <<= 0;
    src.emit( 666 );

    do_transaction( ctx, [&]() {
        timestamp <<= "31 Feb 2021";
        n <<= 1;

        src( 2 );
        src( 7 );
    } );

    const std::vector<record_t> expected = {
        { "1 Jan 2020", -1 },
        { "1 Jan 2020", -1 },
        { "31 Feb 2021", 2 },
        { "31 Feb 2021", 7 },
    };
    CHECK( result.get() == expected );
}

// squaring integer events
TEST_CASE( "Transform" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> squared;
    const auto square = []( auto i ) { return i * i; };

    SUBCASE( "Functional syntax" )
    {
        squared = ureact::transform( src, square );
    }
    SUBCASE( "Piped syntax" )
    {
        squared = src | ureact::transform( square );
    }

    const auto result = ureact::collect<std::vector>( squared );

    for( int i = 0; i < 5; ++i )
        src << i;

    const std::vector<int> expected = { 0, 1, 4, 9, 16 };
    CHECK( result.get() == expected );
}

// clamp integer events with limit which values are in range which is presented in the form
// of signals
TEST_CASE( "TransformSynced" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto limit_range_begin = ureact::make_var( ctx, 4 );
    auto limit_range_size = ureact::make_var( ctx, 4 );

    // make deep dependent signals
    // so they are calculated later than not synced event filter
    auto limit_min = make_deeper( limit_range_begin );
    auto limit_max = make_deeper( limit_range_begin + limit_range_size - 1 );

    ureact::events<int> clamped;
    const auto clamp = []( auto i, int min, int max ) { return std::clamp( i, min, max ); };

    SUBCASE( "Functional syntax" )
    {
        clamped = ureact::transform( src, with( limit_min, limit_max ), clamp );
    }
    // todo: Piped syntax is not yet supported for synced version
    //    SUBCASE( "Piped syntax" )
    //    {
    //        clamped = src | ureact::transform( with( limit_min, limit_max ), clamp );
    //    }

    const auto result = ureact::collect<std::vector>( clamped );

    // make not synced analog to show the difference
    const auto clamp_not_synced
        = [&]( auto i ) { return std::clamp( i, limit_min.get(), limit_max.get() ); };
    ureact::events<int> clamped_not_synced = ureact::transform( src, clamp_not_synced );
    const auto result_not_synced = ureact::collect<std::vector>( clamped_not_synced );

    for( int i : { -1, 4, 10, 0, 5, 2 } )
        src << i;

    // change limits and pass the same values second time
    do_transaction( ctx, [&]() {
        for( int i : { -1, 4, 10, 0, 5, 2 } )
            src << i;

        limit_range_begin <<= 1;
        limit_range_size <<= 3;
    } );

    // we expect only numbers in [limit_min, limit_max] range passed our filter
    // synced filtering performed only after new limit values are calculated
    const std::vector<int> expected
        = { /*first range*/ 4, 4, 7, 4, 5, 4, /*second range*/ 1, 3, 3, 1, 3, 2 };
    CHECK( result.get() == expected );

    // we expect that second pass would use the old limit values because they are not recalculated yet
    const std::vector<int> expected_not_synced
        = { /*first range*/ 4, 4, 7, 4, 5, 4, /*second range*/ 4, 4, 7, 4, 5, 4 };
    CHECK( result_not_synced.get() == expected_not_synced );
}

// filter only even integer events
// our check function depends only on event value
TEST_CASE( "Filter" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> filtered;
    const auto is_even = []( auto i ) { return i % 2 == 0; };

    SUBCASE( "Functional syntax" )
    {
        filtered = ureact::filter( src, is_even );
    }
    SUBCASE( "Piped syntax" )
    {
        filtered = src | ureact::filter( is_even );
    }

    const auto result = ureact::collect<std::vector>( filtered );

    for( int i = 0; i < 10; ++i )
        src << i;

    // we expect only even numbers passed our filter
    const std::vector<int> expected = { 0, 2, 4, 6, 8 };
    CHECK( result.get() == expected );
}

// filter integer events which values are in range which is presented in the form
// of signals
TEST_CASE( "FilterSynced" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto limit_range_begin = ureact::make_var( ctx, 4 );
    auto limit_range_size = ureact::make_var( ctx, 4 );

    // make deep dependent signals
    // so they are calculated later than not synced event filter
    auto limit_min = make_deeper( limit_range_begin );
    auto limit_max = make_deeper( limit_range_begin + limit_range_size - 1 );

    ureact::events<int> filtered;
    const auto in_range = []( auto i, int min, int max ) { return i >= min && i <= max; };

    SUBCASE( "Functional syntax" )
    {
        filtered = ureact::filter( src, with( limit_min, limit_max ), in_range );
    }
    // todo: Piped syntax is not yet supported for synced version
    //    SUBCASE( "Piped syntax" )
    //    {
    //        filtered = src | ureact::filter( with( limit_min, limit_max ), in_range );
    //    }

    const auto result = ureact::collect<std::vector>( filtered );

    // make not synced analog to show the difference
    const auto in_range_not_synced
        = [&]( auto i ) { return i >= limit_min.get() && i <= limit_max.get(); };
    ureact::events<int> filtered_not_synced = ureact::filter( src, in_range_not_synced );
    const auto result_not_synced = ureact::collect<std::vector>( filtered_not_synced );

    for( int i = 0; i < 10; ++i )
        src << i;

    // change limits and pass the same values second time
    do_transaction( ctx, [&]() {
        for( int i = 0; i < 10; ++i )
            src << i;

        limit_range_begin <<= 1;
        limit_range_size <<= 3;
    } );

    // we expect only numbers in [limit_min, limit_max] range passed our filter
    // synced filtering performed only after new limit values are calculated
    const std::vector<int> expected = { /*first range*/ 4, 5, 6, 7, /*second range*/ 1, 2, 3 };
    CHECK( result.get() == expected );

    // we expect that second pass would use the old limit values because they are not recalculated yet
    const std::vector<int> expected_not_synced
        = { /*first range*/ 4, 5, 6, 7, /*second range*/ 4, 5, 6, 7 };
    CHECK( result_not_synced.get() == expected_not_synced );
}

// filters that take first N elements or skip first N elements
TEST_CASE( "TakeOrDropN" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> first_n;
    ureact::events<int> without_first_n;

    SUBCASE( "Functional syntax" )
    {
        first_n = ureact::take( src, 5 );
        without_first_n = ureact::drop( src, 5 );
    }
    SUBCASE( "Piped syntax" )
    {
        first_n = src | ureact::take( 5 );
        without_first_n = src | ureact::drop( 5 );
    }

    const auto result_first_n = ureact::collect<std::vector>( first_n );
    const auto result_without_first_n = ureact::collect<std::vector>( without_first_n );

    // pass integers as events
    for( int i : { 0, 1, 2, 3, 4, 5, -1, 6, 7, 8, 9 } )
        src << i;

    // if we concatenate results of take(N) and drop(N) we receive original set of events
    const std::vector<int> expected_first_n = //
        { 0, 1, 2, 3, 4 };
    const std::vector<int> expected_without_first_n = //
        /*          */ { 5, -1, 6, 7, 8, 9 };
    CHECK( result_first_n.get() == expected_first_n );
    CHECK( result_without_first_n.get() == expected_without_first_n );
}

// take only first event from the source
TEST_CASE( "Once" )
{
    ureact::context ctx;

    const auto is_negative = [&]( auto i ) { return i < 0; };

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> negatives = ureact::filter( src, is_negative );
    ureact::events<int> first;
    ureact::events<int> first_negative;

    SUBCASE( "Functional syntax" )
    {
        first = ureact::once( src );
        first_negative = ureact::once( negatives );
    }
    SUBCASE( "Piped syntax" )
    {
        first = src | ureact::once();
        first_negative = negatives | ureact::once();
    }

    const auto result_first = ureact::collect<std::vector>( first );
    const auto result_first_negative = ureact::collect<std::vector>( first_negative );

    // pass integers as events
    for( int i : { 5, 1, 2, 4, -6, 0, -2 } )
        src << i;

    const std::vector<int> expected_first = { 5 };
    CHECK( result_first.get() == expected_first );

    const std::vector<int> expected_first_negative = { -6 };
    CHECK( result_first_negative.get() == expected_first_negative );
}

// filters that take first elements or skip first elements according to given predicate
TEST_CASE( "TakeOrDropWhile" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> before_negative;
    ureact::events<int> from_negative;

    const auto is_not_negative = [&]( auto i ) { return i >= 0; };

    SUBCASE( "Functional syntax" )
    {
        before_negative = ureact::take_while( src, is_not_negative );
        from_negative = ureact::drop_while( src, is_not_negative );
    }
    SUBCASE( "Piped syntax" )
    {
        before_negative = src | ureact::take_while( is_not_negative );
        from_negative = src | ureact::drop_while( is_not_negative );
    }

    const auto result_before_negative = ureact::collect<std::vector>( before_negative );
    const auto result_from_negative = ureact::collect<std::vector>( from_negative );

    // pass integers as events
    for( int i : { 0, 1, 2, 3, 4, 5, -1, 6, 7, 8, 9 } )
        src << i;

    // if we concatenate results of take_while(pred) and drop_while(pred) we receive original vector
    const std::vector<int> expected_before_negative = //
        { 0, 1, 2, 3, 4, 5 };
    const std::vector<int> expected_from_negative = //
        /*             */ { -1, 6, 7, 8, 9 };
    CHECK( result_before_negative.get() == expected_before_negative );
    CHECK( result_from_negative.get() == expected_from_negative );
}

// filters that take first elements or skip first elements according to given predicate
// that depends on signal values
TEST_CASE( "TakeOrDropWhileSynced" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> before_overflow;
    ureact::events<int> from_overflow;

    const ureact::var_signal<int> blackjack = make_var( ctx, 21 );
    const ureact::signal<int> sum = ureact::fold( src, 0, std::plus<>() );
    const ureact::signal<bool> overflowed = sum >= blackjack;

    const auto is_not_overflowed
        = []( int /*e*/, int sum_value, int blackjack ) { return sum_value < blackjack; };
    const auto is_not_overflowed_2 = []( int /*e*/, bool overflowed ) { return !overflowed; };

    SUBCASE( "Functional syntax" )
    {
        before_overflow = ureact::take_while( src, with( sum, blackjack ), is_not_overflowed );
        from_overflow = ureact::drop_while( src, with( sum, blackjack ), is_not_overflowed );
    }
    // todo: Piped syntax is not yet supported for synced version
    //    SUBCASE( "Piped syntax" )
    //    {
    //        before_overflow = src | ureact::take_while( with(sum, blackjack), is_not_overflowed );
    //        from_overflow = src | ureact::drop_while( with(sum, blackjack), is_not_overflowed );
    //    }
    SUBCASE( "Calculated bool condition" )
    {
        before_overflow = ureact::take_while( src, with( overflowed ), is_not_overflowed_2 );
        from_overflow = ureact::drop_while( src, with( overflowed ), is_not_overflowed_2 );
    }

    const auto result_before_overflow = ureact::collect<std::vector>( before_overflow );
    const auto result_from_overflow = ureact::collect<std::vector>( from_overflow );

    // pass integers as events
    for( int i : { 10, 5, 1, 3 /*19*/, 6 /*25*/, 4, 1, 11 } )
        src << i;

    // if we concatenate results of take_while(pred) and drop_while(pred) we receive original vector
    const std::vector<int> expected_before_overflow = //
        { 10, 5, 1, 3 };
    const std::vector<int> expected_from_overflow = //
        /*       */ { 6, 4, 1, 11 };
    CHECK( result_before_overflow.get() == expected_before_overflow );
    CHECK( result_from_overflow.get() == expected_from_overflow );
}

// demonstrate and test special filter function unique
TEST_CASE( "Unique" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> uniq;

    SUBCASE( "Functional syntax" )
    {
        uniq = ureact::unique( src );
    }
    SUBCASE( "Piped syntax" )
    {
        uniq = src | ureact::unique();
    }

    const auto result = ureact::collect<std::vector>( uniq );

    // pass set containing several duplicate elements
    for( int i : { 1, 2, 1, 1, 3, 3, 3, 4, 5, 4 } )
        src << i;

    // expect removing consecutive (adjacent) duplicates
    const std::vector<int> expected = { 1, 2, 1, 3, 4, 5, 4 };
    CHECK( result.get() == expected );
}

// Counts amount of received events into signal<S>
TEST_CASE( "Count" )
{
    ureact::context ctx;

    auto src = ureact::make_source<>( ctx );
    ureact::signal<size_t> counter;
    ureact::signal<int> integer_counter;

    SUBCASE( "Functional syntax" )
    {
        counter = ureact::count( src );              // Default version creates signal<size_t>
        integer_counter = ureact::count<int>( src ); // Type of signal can be explicitly requested
    }
    SUBCASE( "Piped syntax" )
    {
        counter = src | ureact::count();
        integer_counter = src | ureact::count<int>();
    }

    src.emit();
    src.emit();

    CHECK( counter.get() == 2 );
    CHECK( integer_counter.get() == 2 );
}

// Collects received events into signal<ContT<E>>
TEST_CASE( "Collect" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::signal<std::vector<int>> collected_vec;
    ureact::signal<std::deque<int>> collected_deq;
    ureact::signal<std::list<int>> collected_lst;
    ureact::signal<std::set<int>> collected_set;

    SUBCASE( "Functional syntax" )
    {
        collected_vec = ureact::collect<std::vector>( src );
        collected_deq = ureact::collect<std::deque>( src );
        collected_lst = ureact::collect<std::list>( src );
        collected_set = ureact::collect<std::set>( src );
    }
    SUBCASE( "Piped syntax" )
    {
        collected_vec = src | ureact::collect<std::vector>();
        collected_deq = src | ureact::collect<std::deque>();
        collected_lst = src | ureact::collect<std::list>();
        collected_set = src | ureact::collect<std::set>();
    }

    for( int i : { 1, 2, 3 } )
        src << i;

    CHECK_EQ( collected_vec.get(), std::vector{ 1, 2, 3 } );
    CHECK_EQ( collected_deq.get(), std::deque{ 1, 2, 3 } );
    CHECK_EQ( collected_lst.get(), std::list{ 1, 2, 3 } );
    CHECK_EQ( collected_set.get(), std::set{ 1, 2, 3 } );
}

// hold the last event value as a signal
TEST_CASE( "Hold" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::signal<int> held;

    SUBCASE( "Functional syntax" )
    {
        held = ureact::hold( src, -1 );
    }
    SUBCASE( "Piped syntax" )
    {
        held = src | ureact::hold( -1 );
    }

    int changes = 0;
    observe( held, [&changes]( const auto& ) { ++changes; } );

    // pass values into src
    for( int i : { 0, 0, 1, 1, 2, 3 } )
    {
        src << i;
        CHECK( held.get() == i );
    }

    CHECK( changes == 4 );
}

// sets signal value to value of other signal when event is received
TEST_CASE( "Snapshot" )
{
    ureact::context ctx;

    auto trigger = ureact::make_source( ctx );
    auto target = ureact::make_var<int>( ctx, -1 );
    ureact::signal<int> snap;

    SUBCASE( "Functional syntax" )
    {
        snap = ureact::snapshot( trigger, target );
    }
    SUBCASE( "Piped syntax" )
    {
        snap = trigger | ureact::snapshot( target );
    }

    SUBCASE( "Trigger can be any type" )
    {
        snap = ureact::transform( trigger, []( ureact::token ) { return 1; } )
             | ureact::snapshot( target );
    }

    int changes = 0;
    observe( snap, [&changes]( const auto& ) { ++changes; } );

    for( int i = 0; i < 10; ++i )
    {
        // ensure changing the target do not change the out
        target <<= i;
        assert( snap.get() != target.get() );

        // ensure emitting the trigger changes the out
        trigger();
        assert( snap.get() == i );

        // additionally trigger to ensure it does not add additional changes
        for( int j = 0; j < 3; ++j )
            trigger();
    }

    CHECK( changes == 10 );
}

// Emits value of target signal when event is received
TEST_CASE( "Pulse" )
{
    ureact::context ctx;

    auto trigger = ureact::make_source( ctx );
    auto target = ureact::make_var<int>( ctx, -1 );
    ureact::events<int> beat;

    SUBCASE( "Functional syntax" )
    {
        beat = ureact::pulse( trigger, target );
    }
    SUBCASE( "Piped syntax" )
    {
        beat = trigger | ureact::pulse( target );
    }

    SUBCASE( "Trigger can be any type" )
    {
        beat = ureact::transform( trigger, []( ureact::token ) { return 1; } )
             | ureact::pulse( target );
    }

    auto result = ureact::collect<std::vector>( beat );

    target <<= 1;
    for( int i = 0; i < 2; ++i )
        trigger();

    do_transaction( ctx, [&]() {
        target <<= 6;
        for( int i = 0; i < 3; ++i )
            trigger();
    } );

    // we expect first value twice and second value thrice
    const std::vector<int> expected = { 1, 1, 6, 6, 6 };
    CHECK( result.get() == expected );
}

// Emits value changes of signal as events
TEST_CASE( "Monitor" )
{
    ureact::context ctx;

    auto src = ureact::make_var<int>( ctx, -1 );
    ureact::events<int> monitored;
    ureact::events<> changes;
    ureact::events<> changes_to_zero;

    // there are two syntax variants, so we check them all
    // using subcases
    SUBCASE( "Functional syntax" )
    {
        monitored = ureact::monitor( src );
        changes = ureact::changed( src );
        changes_to_zero = ureact::changed_to( src, 0 );
    }
    SUBCASE( "Piped syntax" )
    {
        monitored = src | ureact::monitor();
        changes = src | ureact::changed();
        changes_to_zero = src | ureact::changed_to( 0 );
    }

    const auto result = ureact::collect<std::vector>( monitored );
    const auto changes_count = count( changes );
    const auto changes_to_zero_count = count( changes_to_zero );

    // pass values into src
    for( int i : { 0, 0, 0, 1, 1, 2, 3, 0 } )
        src <<= i;

    const std::vector<int> expected = { 0, 1, 2, 3, 0 };
    CHECK( result.get() == expected );
    CHECK( changes_count.get() == 5 );
    CHECK( changes_to_zero_count.get() == 2 );
}
