//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/closure.hpp"

#include "doctest_extra.h"
#include "ureact/collect.hpp"
#include "ureact/filter.hpp"
#include "ureact/transform.hpp"
#include "ureact/ureact.hpp"

/*!
 *  Closure objects take one reactive object as its only argument and may return a value.
 *  They are callable via the pipe operator: if C is a closure object and
 *  R is a reactive object, these two expressions are equivalent:
 *  * C(R)
 *  * R | C
 *
 * @note similar to https://en.cppreference.com/w/cpp/ranges#Range_adaptor_closure_objects
 */
TEST_CASE( "ClosureCall" )
{
    ureact::context ctx;
    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> e;

    const auto is_positive = []( auto i ) { return i > 0; };

#define R src
#define C ureact::filter( is_positive )

    SUBCASE( "functional call" )
    {
        e = C( R );
    }
    SUBCASE( "pipe operator" )
    {
        e = R | C;
    }

#undef R
#undef C

    auto result = ureact::collect<std::vector>( e );

    for( int i : { -1, 4, -10, 0, 5, 2 } )
        src << i;

    CHECK( result.get() == std::vector<int>{ 4, 5, 2 } );
}

/*!
 *  Two closure objects can be chained by operator| to produce
 *  another closure object: if C and D are closure objects,
 *  then C | D is also a closure object if it is valid.
 *  The effect and validity of the operator() of the result is determined as follows:
 *  given a reactive object R, these two expressions are equivalent:
 *  * R | C | D // (R | C) | D
 *  * R | (C | D)
 *
 * @note similar to https://en.cppreference.com/w/cpp/ranges#Range_adaptor_closure_objects
 */
TEST_CASE( "ClosureChaining" )
{
    ureact::context ctx;
    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> e;

    const auto is_positive = []( auto i ) { return i > 0; };
    const auto square = []( auto i ) { return i * i; };

#define R src
#define C ureact::filter( is_positive )
#define D ureact::transform( square )

    SUBCASE( "default composition" )
    {
        e = R | C | D;
    }
    SUBCASE( "default composition emphasized" )
    {
        e = ( R | C ) | D;
    }
    SUBCASE( "closure composition" )
    {
        e = R | ( C | D );
    }
    SUBCASE( "closure composition L-value" )
    {
        ureact::closure chained = C | D;
        e = R | chained;
    }

#undef R
#undef C
#undef D

    auto result = ureact::collect<std::vector>( e );

    for( int i : { -1, 4, -10, 0, 5, 2 } )
        src << i;

    CHECK( result.get() == std::vector<int>{ 16, 25, 4 } );
}
