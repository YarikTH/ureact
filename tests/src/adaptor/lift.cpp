//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/lift.hpp"

#include <functional>

#include "catch2_extra.hpp"
#include "copy_stats.hpp"
#include "identity.hpp"
#include "ureact/adaptor/observe.hpp"
#include "ureact/transaction.hpp"

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
TEST_CASE( "ureact::lift (invoke)" )
{
    ureact::context ctx;

    SECTION( "invoke a free function" )
    {
        ureact::var_signal<int> src = ureact::make_var( ctx, 4 );

        CHECK( std::invoke( double_int, src.get() ) == 8 );

        auto negated = ureact::lift( src, double_int );

        CHECK( negated.get() == 8 );
    }

    SECTION( "invoke a lambda" )
    {
        ureact::var_signal<int> src = ureact::make_var( ctx, 3 );

        auto squared = []( auto x ) { return x * x; };

        CHECK( std::invoke( squared, src.get() ) == 9 );

        auto negated = ureact::lift( src, squared );

        CHECK( negated.get() == 9 );
    }

    SECTION( "invoke a function object" )
    {
        ureact::var_signal<int> src = ureact::make_var( ctx, 1 );

        CHECK( std::invoke( std::negate<>{}, src.get() ) == -1 );

        auto negated = ureact::lift( src, std::negate<>{} );

        CHECK( negated.get() == -1 );
    }

    SECTION( "invoke a member function (value)" )
    {
        ureact::var_signal src = ureact::make_var<User>( ctx, 18 );
        auto _3 = ureact::make_const( ctx, 3 );

        CHECK( std::invoke( &User::get_age, src.get() ) == 18 );
        CHECK( std::invoke( &User::add_age, src.get(), _3.get() ) == 21 );

        auto age = ureact::lift( src, &User::get_age );
        auto sum = ureact::lift( with( src, _3 ), &User::add_age );

        CHECK( age.get() == 18 );
        CHECK( sum.get() == 21 );
    }

    SECTION( "access a data member (value)" )
    {
        ureact::var_signal src = ureact::make_var<User>( ctx, 32 );

        CHECK( std::invoke( &User::age, src.get() ) == 32 );

        auto age = ureact::lift( src, &User::age );

        CHECK( age.get() == 32 );
    }

    SECTION( "invoke a member function (pointer)" )
    {
        User user{ 18 };

        ureact::var_signal<User*> src = ureact::make_var( ctx, &user );
        auto _3 = ureact::make_const( ctx, 3 );

        CHECK( std::invoke( &User::get_age, src.get() ) == 18 );
        CHECK( std::invoke( &User::add_age, src.get(), _3.get() ) == 21 );

        auto age = ureact::lift( src, &User::get_age );
        auto sum = ureact::lift( with( src, _3 ), &User::add_age );

        CHECK( age.get() == 18 );
        CHECK( sum.get() == 21 );
    }

    SECTION( "access a data member (pointer)" )
    {
        User user{ 32 };

        ureact::var_signal<User*> src = ureact::make_var( ctx, &user );

        CHECK( std::invoke( &User::age, src.get() ) == 32 );

        auto age = ureact::lift( src, &User::age );

        CHECK( age.get() == 32 );
    }

    // TODO: check member functions and data member access for Foo& and Foo* types
}

TEST_CASE( "ureact::lift (unary)" )
{
    ureact::context ctx;

    ureact::var_signal src = make_var( ctx, 1 );
    ureact::temp_signal tmp = ureact::lift( src, []( auto i ) { return i * 2; } );
    ureact::signal<int> result_1;
    ureact::signal<int> result_2;

    CHECK_FALSE( tmp.was_op_stolen() );

    SECTION( "Overloaded operator" )
    {
        result_1 = -src;
        result_2 = -std::move( tmp );
    }
    SECTION( "Functional syntax" )
    {
        result_1 = ureact::lift( src, std::negate<>{} );
        result_2 = ureact::lift( std::move( tmp ), std::negate<>{} );
    }
    SECTION( "Piped syntax" )
    {
        result_1 = src | ureact::lift( std::negate<>{} );
        result_2 = std::move( tmp ) | ureact::lift( std::negate<>{} );
    }

    CHECK( tmp.was_op_stolen() );

    CHECK( result_1.get() == -1 );
    CHECK( result_2.get() == -2 );

    src <<= -7;

    CHECK( result_1.get() == 7 );
    CHECK( result_2.get() == 14 );
}

TEST_CASE( "ureact::lift (binary)" )
{
    ureact::context ctx;

    ureact::var_signal lhs = make_var( ctx, 10 );
    ureact::temp_signal lhs_tmp_1 = ureact::lift( lhs, identity{} );
    ureact::temp_signal lhs_tmp_2 = ureact::lift( lhs, identity{} );
    ureact::temp_signal lhs_tmp_3 = ureact::lift( lhs, identity{} );

    ureact::var_signal rhs = make_var( ctx, 3 );
    ureact::temp_signal rhs_tmp_1 = ureact::lift( rhs, identity{} );
    ureact::temp_signal rhs_tmp_2 = ureact::lift( rhs, identity{} );
    ureact::temp_signal rhs_tmp_3 = ureact::lift( rhs, identity{} );

    ureact::signal<int> const_op_signal;
    ureact::signal<int> const_op_temp;
    ureact::signal<int> signal_op_const;
    ureact::signal<int> signal_op_signal;
    ureact::signal<int> signal_op_temp;
    ureact::signal<int> temp_op_const;
    ureact::signal<int> temp_op_signal;
    ureact::signal<int> temp_op_temp;

    CHECK_FALSE( lhs_tmp_1.was_op_stolen() );
    CHECK_FALSE( lhs_tmp_2.was_op_stolen() );
    CHECK_FALSE( lhs_tmp_3.was_op_stolen() );
    CHECK_FALSE( rhs_tmp_1.was_op_stolen() );
    CHECK_FALSE( rhs_tmp_2.was_op_stolen() );
    CHECK_FALSE( rhs_tmp_3.was_op_stolen() );

    SECTION( "Overloaded operator" )
    {
        // clang-format off
        const_op_signal  = 10                     - rhs;
        const_op_temp    = 10                     - std::move( rhs_tmp_1 );
        signal_op_const  = lhs                    - 3;
        signal_op_signal = lhs                    - rhs;
        signal_op_temp   = lhs                    - std::move( rhs_tmp_2 );
        temp_op_const    = std::move( lhs_tmp_1 ) - 3;
        temp_op_signal   = std::move( lhs_tmp_2 ) - rhs;
        temp_op_temp     = std::move( lhs_tmp_3 ) - std::move( rhs_tmp_3 );
        // clang-format on
    }
    SECTION( "Functional syntax" )
    {
        // clang-format off
        const_op_signal  = ureact::lift( 10                    , std::minus<>{}, rhs );
        const_op_temp    = ureact::lift( 10                    , std::minus<>{}, std::move( rhs_tmp_1 ) );
        signal_op_const  = ureact::lift( lhs                   , std::minus<>{}, 3 );
        signal_op_signal = ureact::lift( lhs                   , std::minus<>{}, rhs );
        signal_op_temp   = ureact::lift( lhs                   , std::minus<>{}, std::move( rhs_tmp_2 ) );
        temp_op_const    = ureact::lift( std::move( lhs_tmp_1 ), std::minus<>{}, 3 );
        temp_op_signal   = ureact::lift( std::move( lhs_tmp_2 ), std::minus<>{}, rhs );
        temp_op_temp     = ureact::lift( std::move( lhs_tmp_3 ), std::minus<>{}, std::move( rhs_tmp_3 ) );
        // clang-format on
    }

    CHECK( lhs_tmp_1.was_op_stolen() );
    CHECK( lhs_tmp_2.was_op_stolen() );
    CHECK( lhs_tmp_3.was_op_stolen() );
    CHECK( rhs_tmp_1.was_op_stolen() );
    CHECK( rhs_tmp_2.was_op_stolen() );
    CHECK( rhs_tmp_3.was_op_stolen() );

    CHECK( const_op_signal.get() == 7 );
    CHECK( const_op_temp.get() == 7 );
    CHECK( signal_op_const.get() == 7 );
    CHECK( signal_op_signal.get() == 7 );
    CHECK( signal_op_temp.get() == 7 );
    CHECK( temp_op_const.get() == 7 );
    CHECK( temp_op_signal.get() == 7 );
    CHECK( temp_op_temp.get() == 7 );

    lhs <<= 20;

    CHECK( const_op_signal.get() == 7 );
    CHECK( const_op_temp.get() == 7 );
    CHECK( signal_op_const.get() == 17 );
    CHECK( signal_op_signal.get() == 17 );
    CHECK( signal_op_temp.get() == 17 );
    CHECK( temp_op_const.get() == 17 );
    CHECK( temp_op_signal.get() == 17 );
    CHECK( temp_op_temp.get() == 17 );

    rhs <<= 5;

    CHECK( const_op_signal.get() == 5 );
    CHECK( const_op_temp.get() == 5 );
    CHECK( signal_op_const.get() == 17 );
    CHECK( signal_op_signal.get() == 15 );
    CHECK( signal_op_temp.get() == 15 );
    CHECK( temp_op_const.get() == 17 );
    CHECK( temp_op_signal.get() == 15 );
    CHECK( temp_op_temp.get() == 15 );
}

TEST_CASE( "ureact::lift (arithmetic operators)" )
{
    ureact::context ctx;

    ureact::var_signal lhs = make_var( ctx, 10 );
    ureact::var_signal rhs = make_var( ctx, 3 );

    CHECK( ( lhs + rhs ).get() == ( 10 + 3 ) );
    CHECK( ( lhs - rhs ).get() == ( 10 - 3 ) );
    CHECK( ( lhs * rhs ).get() == ( 10 * 3 ) );
    CHECK( ( lhs / rhs ).get() == ( 10 / 3 ) );
    CHECK( ( lhs % rhs ).get() == ( 10 % 3 ) );
    CHECK( ( +lhs ).get() == ( +10 ) );
    CHECK( ( -lhs ).get() == ( -10 ) );
}

TEST_CASE( "ureact::lift (relational operators)" )
{
    ureact::context ctx;

    ureact::var_signal lhs = make_var( ctx, -5 );
    ureact::var_signal rhs = make_var( ctx, 25 );

    // clang-format off
    CHECK( ( lhs == rhs ).get() == ( -5 == 25 ) );
    CHECK( ( lhs != rhs ).get() == ( -5 != 25 ) );
    CHECK( ( lhs <  rhs ).get() == ( -5 <  25 ) );
    CHECK( ( lhs <= rhs ).get() == ( -5 <= 25 ) );
    CHECK( ( lhs >  rhs ).get() == ( -5 >  25 ) );
    CHECK( ( lhs >= rhs ).get() == ( -5 >= 25 ) );
    // clang-format on
}

TEST_CASE( "ureact::lift (logical operators)" )
{
    ureact::context ctx;

    ureact::var_signal lhs = make_var( ctx, false );
    ureact::var_signal rhs = make_var( ctx, false );

    for( auto [l, r] : { //
             std::pair{ false, false },
             std::pair{ false, true },
             std::pair{ true, false },
             std::pair{ true, true } } )
    {
        lhs <<= l;
        rhs <<= r;

        CHECK( ( lhs && rhs ).get() == ( l && r ) );
        CHECK( ( lhs || rhs ).get() == ( l || r ) );
        CHECK( ( !lhs ).get() == ( !l ) );
    }
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

    ureact::observer _ = ureact::observe( my_shape.size, on_size_value_change );

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
    SECTION( "using overloaded operators" )
    {
        bothWords = firstWord + " " + secondWord;
    }

    SECTION( "using lift()" )
    {
        bothWords = ureact::lift( with( firstWord, secondWord ), concatFunc );
    }

    SECTION( "operators , and |" )
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

TEST_CASE( "ureact::lift (operator priority)" )
{
    ureact::context ctx;

    auto _2 = make_var( ctx, 2 );

    auto result = _2 + _2 * _2;
    CHECK( result.get() == 6 );

    auto result2 = ( _2 + _2 ) * _2;
    CHECK( result2.get() == 8 );
}

// Check how it ureact::lift works with functions that return T&& or T&
TEST_CASE( "ureact::lift (ref)" )
{
    ureact::context ctx;

    auto src = make_var( ctx, -1 );
    ureact::signal<int> rvalue;
    ureact::signal<int> lvalue;

    SECTION( "Functional syntax" )
    {
        rvalue = ureact::lift( src, identity{} );
        lvalue = ureact::lift( src, identity_ref{} );
    }
    SECTION( "Piped syntax" )
    {
        rvalue = src | ureact::lift( identity{} );
        lvalue = src | ureact::lift( identity_ref{} );
    }

    for( int i = 0; i < 5; ++i )
    {
        src <<= i;
        CHECK( rvalue.get() == i );
        CHECK( lvalue.get() == i );
    }
}

TEST_CASE( "ureact::lift_as" )
{
    ureact::context ctx;

    auto src = make_var( ctx, -1 );
    ureact::signal<int64_t> big_int;

    SECTION( "Functional syntax" )
    {
        big_int = ureact::lift_as<int64_t>( src, identity{} );
    }
    SECTION( "Piped syntax" )
    {
        big_int = src | ureact::lift_as<int64_t>( identity{} );
    }

    for( int i = 0; i < 5; ++i )
    {
        src <<= i;
        CHECK( big_int.get() == i );
    }
}
