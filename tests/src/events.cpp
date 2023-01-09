//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>

#include "doctest_extra.h"
#include "ureact/collect.hpp"
#include "ureact/count.hpp"
#include "ureact/filter.hpp"
#include "ureact/merge.hpp"

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

// not constructive from context
static_assert( !std::is_constructible_v<ureact::event_source<>, ureact::context&> );

// copyable and nothrow movable
static_assert( std::is_copy_constructible_v<ureact::event_source<>> );
static_assert( std::is_copy_assignable_v<ureact::event_source<>> );
static_assert( std::is_move_constructible_v<ureact::event_source<>> );
static_assert( std::is_move_assignable_v<ureact::event_source<>> );
static_assert( std::is_nothrow_move_constructible_v<ureact::event_source<>> );
static_assert( std::is_nothrow_move_assignable_v<ureact::event_source<>> );

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
        ureact::events<> src = ureact::make_source<>( ctx );
        CHECK( src.is_valid() );

        SUBCASE( "copy constructed" )
        {
            ureact::events<> src_copy = src;
            CHECK( src_copy.is_valid() );
            CHECK( src.is_valid() );
        }

        SUBCASE( "move constructed" )
        {
            ureact::events<> src_move = std::move( src );
            CHECK( src_move.is_valid() );
            CHECK_FALSE( src.is_valid() );
        }

        SUBCASE( "copy assignment" )
        {
            ureact::events<> src_copy;
            CHECK_FALSE( src_copy.is_valid() );

            src_copy = src;
            CHECK( src_copy.is_valid() );
            CHECK( src.is_valid() );
            CHECK( src_copy.equal_to( src ) );
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

    // copy and move construction of event_source
    SUBCASE( "copy and move constructed" )
    {
        ureact::event_source src = ureact::make_source<int>( ctx );
        CHECK( src.is_valid() );

        SUBCASE( "copy constructed" )
        {
            ureact::event_source<int> src_copy = src;
            CHECK( src_copy.is_valid() );
            CHECK( src.is_valid() );
            CHECK( src_copy.equal_to( src ) );
        }

        SUBCASE( "move constructed" )
        {
            ureact::event_source<int> src_move = std::move( src );
            CHECK( src_move.is_valid() );
            CHECK_FALSE( src.is_valid() );
        }

        SUBCASE( "copy assignment" )
        {
            ureact::event_source<int> src_copy;
            CHECK_FALSE( src_copy.is_valid() );

            src_copy = src;
            CHECK( src_copy.is_valid() );
            CHECK( src.is_valid() );
            CHECK( src_copy.equal_to( src ) );
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
}

// Events has shared_ptr semantics. They are literally shared_ptr to reactive node
// that does all the work and form dependency tree
TEST_CASE( "EventsSmartPointerSemantics" )
{
    ureact::context ctx;

    ureact::event_source<int> src = ureact::make_source<int>( ctx );

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

TEST_CASE( "EventRange" )
{
    SUBCASE( "empty range" )
    {
        std::vector<int> data;
        ureact::event_range<int> range{ data };

        CHECK( range.empty() );
        CHECK( range.size() == 0 );
        const bool are_begin_end_equal = range.begin() == range.end();
        CHECK( are_begin_end_equal );
        const bool are_rbegin_rend_equal = range.rbegin() == range.rend();
        CHECK( are_rbegin_rend_equal );
    }

    SUBCASE( "non-empty range" )
    {
        std::vector<int> data{ 0, 1, 2, 3, 4 };
        ureact::event_range<int> range{ data };

        CHECK_FALSE( range.empty() );
        CHECK( range.size() == 5 );
        const bool are_begin_end_equal = range.begin() == range.end();
        CHECK_FALSE( are_begin_end_equal );
        const bool are_rbegin_rend_equal = range.rbegin() == range.rend();
        CHECK_FALSE( are_rbegin_rend_equal );

        const auto direct_range_copy = std::vector<int>{ data.begin(), data.end() };
        const auto reverse_range_copy = std::vector<int>{ data.rbegin(), data.rend() };
        CHECK( direct_range_copy == std::vector<int>{ 0, 1, 2, 3, 4 } );
        CHECK( reverse_range_copy == std::vector<int>{ 4, 3, 2, 1, 0 } );
    }
}

TEST_CASE( "EventEmitter" )
{
    std::vector<int> data;
    ureact::event_emitter<int> emitter{ data };
    auto _2 = 2;

    SUBCASE( "operator =" )
    {
        emitter = 1;  // R-value
        emitter = _2; // L-value
    }
    SUBCASE( "stream" )
    {
        emitter << 1   // R-value
                << _2; // L-value
    }
    SUBCASE( "stl iterator" )
    {
        std::generate_n( emitter, 1, [] { return 1; } );                   // R-value
        std::generate_n( emitter, 1, [&]() -> const int& { return _2; } ); // L-value
    }

    CHECK( data == std::vector<int>{ 1, 2 } );
}
