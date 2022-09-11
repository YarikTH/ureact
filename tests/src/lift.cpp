//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/lift.hpp"

#include <functional>

#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// TODO: move reworked lift_operators.cpp here
// TODO: move almost all lift tests from signal.cpp here

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
        //        auto age = ureact::lift( src, &User::age );
        //
        //        CHECK( age.get() == 32 );
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
        //        auto age = ureact::lift( src, &User::age );
        //
        //        CHECK( age.get() == 32 );
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
