//
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/events.hpp"

#include <algorithm>

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/count.hpp"
#include "ureact/adaptor/merge.hpp"

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

TEST_CASE( "ureact::events<E> (construction)" )
{
    ureact::context ctx;

    // default constructed events isn't linked to a reactive node, thus
    // can't be used for anything but for following assignment
    SECTION( "default constructed" )
    {
        ureact::events<> null_evt;
        CHECK_FALSE( null_evt.is_valid() );
    }

    // events can be created via object slicing from event_source object
    SECTION( "slicing" )
    {
        auto src = ureact::make_source<>( ctx );
        ureact::events<> evt = src;

        CHECK( evt.is_valid() );
    }

    // events can be created via make_never call
    SECTION( "make_never" )
    {
        ureact::events evt = ureact::make_never<>( ctx );

        CHECK( evt.is_valid() );
    }

    // events can be created using various algorithms
    SECTION( "algorithm" )
    {
        auto src1 = ureact::make_source<>( ctx );
        auto src2 = ureact::make_source<>( ctx );
        ureact::events<> evt = ureact::merge( src1, src2 );

        CHECK( evt.is_valid() );
    }

    // copy and move construction of events
    SECTION( "copy and move constructed" )
    {
        ureact::events<> src = ureact::make_source<>( ctx );
        CHECK( src.is_valid() );

        SECTION( "copy constructed" )
        {
            ureact::events<> src_copy = src;
            CHECK( src_copy.is_valid() );
            CHECK( src.is_valid() );
        }

        SECTION( "move constructed" )
        {
            ureact::events<> src_move = std::move( src );
            CHECK( src_move.is_valid() );
            CHECK_FALSE( src.is_valid() );
        }

        SECTION( "copy assignment" )
        {
            ureact::events<> src_copy;
            CHECK_FALSE( src_copy.is_valid() );

            src_copy = src;
            CHECK( src_copy.is_valid() );
            CHECK( src.is_valid() );
            CHECK( src_copy.equal_to( src ) );
        }

        SECTION( "move assignment" )
        {
            ureact::events<> src_move;
            CHECK_FALSE( src_move.is_valid() );

            src_move = std::move( src );
            CHECK( src_move.is_valid() );
            CHECK_FALSE( src.is_valid() );
        }
    }
}

TEST_CASE( "ureact::event_source<E> (construction)" )
{
    ureact::context ctx;

    // default constructed event_source isn't linked to a reactive node, thus
    // can't be used for anything but for following assignment
    SECTION( "default constructed" )
    {
        ureact::event_source<> null_src;
        CHECK_FALSE( null_src.is_valid() );
    }

    // event_source can be created via free function semantically close to std::make_shared
    // Event value type E has to be specified explicitly. It would be token if it is omitted
    SECTION( "make_source<>()" )
    {
        auto src = ureact::make_source<>( ctx );
        CHECK( src.is_valid() );
    }

    // copy and move construction of event_source
    SECTION( "copy and move constructed" )
    {
        ureact::event_source src = ureact::make_source<int>( ctx );
        CHECK( src.is_valid() );

        SECTION( "copy constructed" )
        {
            ureact::event_source<int> src_copy = src;
            CHECK( src_copy.is_valid() );
            CHECK( src.is_valid() );
            CHECK( src_copy.equal_to( src ) );
        }

        SECTION( "move constructed" )
        {
            ureact::event_source<int> src_move = std::move( src );
            CHECK( src_move.is_valid() );
            CHECK_FALSE( src.is_valid() );
        }

        SECTION( "copy assignment" )
        {
            ureact::event_source<int> src_copy;
            CHECK_FALSE( src_copy.is_valid() );

            src_copy = src;
            CHECK( src_copy.is_valid() );
            CHECK( src.is_valid() );
            CHECK( src_copy.equal_to( src ) );
        }

        SECTION( "move assignment" )
        {
            ureact::event_source<int> src_move;
            CHECK_FALSE( src_move.is_valid() );

            src_move = std::move( src );
            CHECK( src_move.is_valid() );
            CHECK_FALSE( src.is_valid() );
        }
    }
}

// We can emit events using a bunch of methods doing basically the same
TEST_CASE( "ureact::make_source<E> (emit)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto _2 = 2;

    auto result = ureact::collect<std::vector>( src );

    SECTION( "emit method" )
    {
        src.emit( 1 );  // R-value
        src.emit( _2 ); // L-value
    }
    SECTION( "function object" )
    {
        src( 1 );  // R-value
        src( _2 ); // L-value
    }
    SECTION( "stream" )
    {
        src << 1   // R-value
            << _2; // L-value
    }
    SECTION( "stl iterator" )
    {
        std::generate_n( src.begin(), 1, [] { return 1; } );                   // R-value
        std::generate_n( src.begin(), 1, [&]() -> const int& { return _2; } ); // L-value
    }

    CHECK( result.get() == std::vector{ 1, 2 } );
}

// We can emit ureact::unit using a bunch of methods doing basically the same
TEST_CASE( "ureact::make_source<ureact::unit> (emit)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<>( ctx );
    auto unit = ureact::unit{};

    auto counted = ureact::count( src );

    SECTION( "emit method" )
    {
        src.emit();                 // event_source<token> specialization without argument
        src.emit( ureact::unit{} ); // R-value
        src.emit( unit );           // L-value
    }
    SECTION( "function object" )
    {
        src();                 // event_source<token> specialization without argument
        src( ureact::unit{} ); // R-value
        src( unit );           // L-value
    }
    SECTION( "stream" )
    {
        src << ureact::unit{} // R-value
            << unit           // L-value
            << unit;          // L-value
    }
    SECTION( "stl iterator" )
    {
        std::generate_n( src.begin(), 1, [] { return ureact::unit{}; } );           // R-value
        std::generate_n( src.begin(), 2, [&]() -> ureact::unit& { return unit; } ); // L-value
    }

    CHECK( counted.get() == 3 );
}
