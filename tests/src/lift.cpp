//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/lift.hpp"

#include <functional>

#include "copy_stats.hpp"
#include "doctest_extra.h"
#include "identity.hpp"
#include "ureact/observe.hpp"
#include "ureact/transaction.hpp"
#include "ureact/ureact.hpp"

// TODO: move reworked lift_operators.cpp here

namespace
{

int double_int( int i )
{
    return i * 2;
}

struct User
{
    User( int age )
        : age( age )
    {}

    // no args member function to check invoke
    [[nodiscard]] int get_age() const
    {
        return age;
    }

    // member function with args to check invoke
    [[nodiscard]] int add_age( int i ) const
    {
        return age + i;
    }

    // public member variable to check invoke
    int age;
};

} // namespace

// See capabilities of std::invoke
// https://en.cppreference.com/w/cpp/utility/functional/invoke
TEST_CASE( "LiftAsInvoke" )
{
    ureact::context ctx;

    SUBCASE( "invoke a free function" )
    {
        ureact::var_signal<int> src{ ctx, 4 };

        CHECK( std::invoke( double_int, src.get() ) == 8 );

        auto negated = ureact::lift( src, double_int );

        CHECK( negated.get() == 8 );
    }

    SUBCASE( "invoke a lambda" )
    {
        ureact::var_signal<int> src{ ctx, 3 };

        auto squared = []( auto x ) { return x * x; };

        CHECK( std::invoke( squared, src.get() ) == 9 );

        auto negated = ureact::lift( src, squared );

        CHECK( negated.get() == 9 );
    }

    SUBCASE( "invoke a function object" )
    {
        ureact::var_signal<int> src{ ctx, 1 };

        CHECK( std::invoke( std::negate<>{}, src.get() ) == -1 );

        auto negated = ureact::lift( src, std::negate<>{} );

        CHECK( negated.get() == -1 );
    }

    SUBCASE( "invoke a member function (value)" )
    {
        ureact::var_signal<User> src{ ctx, 18 };
        auto _3 = ureact::make_const( ctx, 3 );

        CHECK( std::invoke( &User::get_age, src.get() ) == 18 );
        CHECK( std::invoke( &User::add_age, src.get(), _3.get() ) == 21 );

        auto age = ureact::lift( src, &User::get_age );
        auto sum = ureact::lift( with( src, _3 ), &User::add_age );

        CHECK( age.get() == 18 );
        CHECK( sum.get() == 21 );
    }

    SUBCASE( "access a data member (value)" )
    {
        ureact::var_signal<User> src{ ctx, 32 };

        CHECK( std::invoke( &User::age, src.get() ) == 32 );

        // TODO: invoke result is int&& that is not supported yet
        auto age = ureact::lift<int>( src, &User::age );

        CHECK( age.get() == 32 );
    }

    SUBCASE( "invoke a member function (pointer)" )
    {
        User user{ 18 };

        ureact::var_signal<User*> src{ ctx, &user };
        auto _3 = ureact::make_const( ctx, 3 );

        CHECK( std::invoke( &User::get_age, src.get() ) == 18 );
        CHECK( std::invoke( &User::add_age, src.get(), _3.get() ) == 21 );

        auto age = ureact::lift( src, &User::get_age );
        auto sum = ureact::lift( with( src, _3 ), &User::add_age );

        CHECK( age.get() == 18 );
        CHECK( sum.get() == 21 );
    }

    SUBCASE( "access a data member (pointer)" )
    {
        User user{ 32 };

        ureact::var_signal<User*> src{ ctx, &user };

        CHECK( std::invoke( &User::age, src.get() ) == 32 );

        // TODO: invoke result is int& that is not supported yet
        auto age = ureact::lift<int>( src, &User::age );

        CHECK( age.get() == 32 );
    }

    // TODO: check member functions and data member access for Foo& and Foo* types
}

// TODO: prevent such behaviour. Possibly by forcing constness of a pointer
TEST_CASE( "NonConstPointerBug" )
{
    ureact::context ctx;

    int i = 42;

    ureact::var_signal<int*> src{ ctx, &i };

    auto wtf = ureact::lift( src, []( int* p ) { return *p = -1; } );
    CHECK( i == -1 );
    CHECK( wtf.get() == -1 );
}

TEST_CASE( "LiftUnary" )
{
    ureact::context ctx;

    ureact::var_signal v1 = make_var( ctx, 1 );
    ureact::temp_signal v2 = ureact::lift( v1, []( auto i ) { return i * 2; } );
    ureact::signal<int> minus_v1;
    ureact::signal<int> minus_v2;

    CHECK_FALSE( v2.was_op_stolen() );

    SUBCASE( "Overloaded operator" )
    {
        minus_v1 = -v1;
        minus_v2 = -std::move( v2 );
    }
    SUBCASE( "Functional syntax" )
    {
        minus_v1 = ureact::lift( v1, std::negate<>{} );
        minus_v2 = ureact::lift( std::move( v2 ), std::negate<>{} );
    }
    SUBCASE( "Piped syntax" )
    {
        minus_v1 = v1 | ureact::lift( std::negate<>{} );
        minus_v2 = std::move( v2 ) | ureact::lift( std::negate<>{} );
    }

    CHECK( v2.was_op_stolen() );

    CHECK( minus_v1.get() == -1 );
    CHECK( minus_v2.get() == -2 );

    v1 <<= -7;

    CHECK( minus_v1.get() == 7 );
    CHECK( minus_v2.get() == 14 );
}

TEST_CASE( "Reactive class members" )
{
    ureact::context ctx;

    class Shape
    {
    public:
        explicit Shape( ureact::context& ctx )
            : width( make_var( ctx, 0 ) )
            , height( make_var( ctx, 0 ) )
            , size( width * height )
        {}

        ureact::var_signal<int> width;
        ureact::var_signal<int> height;

        ureact::signal<int> size;
    };

    Shape my_shape( ctx );

    CHECK( my_shape.width.get() == 0 );
    CHECK( my_shape.height.get() == 0 );
    CHECK( my_shape.size.get() == 0 );

    std::vector<int> size_values;

    auto on_size_value_change = [&]( const int new_value ) { size_values.push_back( new_value ); };

    observe( my_shape.size, on_size_value_change );

    CHECK( size_values == std::vector<int>{} );

    // Do transaction to change width and height in single step
    do_transaction( ctx, [&] {
        my_shape.width <<= 4;
        my_shape.height <<= 4;
    } );

    CHECK( size_values == std::vector<int>{ 16 } );

    CHECK( my_shape.width.get() == 4 );
    CHECK( my_shape.height.get() == 4 );
    CHECK( my_shape.size.get() == 16 );
}

TEST_CASE( "Hello World" )
{
    ureact::context ctx;

    // make_var is available as a free function and as context's member function
    ureact::var_signal<std::string> firstWord = make_var( ctx, std::string( "Change" ) );
    ureact::var_signal<std::string> secondWord = make_var( ctx, std::string( "me!" ) );

    ureact::signal<std::string> bothWords;

    auto concatFunc = []( const std::string& first, const std::string& second ) -> std::string {
        return first + " " + second;
    };

    // Several alternative variants that do exactly the same
    SUBCASE( "using overloaded operators" )
    {
        bothWords = firstWord + " " + secondWord;
    }

    SUBCASE( "using lift()" )
    {
        bothWords = lift( with( firstWord, secondWord ), concatFunc );
    }

    SUBCASE( "operators , and |" )
    {
        // operator "|" can be used instead of lift()
        bothWords = with( firstWord, secondWord ) | ureact::lift( concatFunc );
    }

    // Imperative value access
    CHECK( bothWords.get() == "Change me!" );

    // Imperative value change
    firstWord <<= std::string( "Hello" );

    CHECK( bothWords.get() == "Hello me!" );

    secondWord <<= std::string( "World!" );

    CHECK( bothWords.get() == "Hello World!" );
}

TEST_CASE( "CopyStatsForSignalCalculations" )
{
    ureact::context ctx;

    copy_stats stats;

    auto a = ureact::make_var( ctx, copy_counter{ 1, &stats } );
    auto b = ureact::make_var( ctx, copy_counter{ 10, &stats } );
    auto c = ureact::make_var( ctx, copy_counter{ 100, &stats } );
    auto d = ureact::make_var( ctx, copy_counter{ 1000, &stats } );

    // 4x move to m_value
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

TEST_CASE( "LiftOperatorPriority" )
{
    ureact::context ctx;

    auto _2 = make_var( ctx, 2 );

    auto result = _2 + _2 * _2;
    CHECK( result.get() == 6 );

    auto result2 = ( _2 + _2 ) * _2;
    CHECK( result2.get() == 8 );
}
