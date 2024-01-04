//
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/filter.hpp"
#include "ureact/adaptor/lift.hpp"
#include "ureact/events.hpp"
#include "ureact/signal.hpp"

// Signal has shared_ptr semantics. They are literally shared_ptr to reactive node
// that does all the work and form dependency tree
TEST_CASE( "ureact::signal<S> (smart pointer semantics)" )
{
    ureact::context ctx;

    ureact::var_signal x = ureact::make_var( ctx, 1 );

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

// Events has shared_ptr semantics. They are literally shared_ptr to reactive node
// that does all the work and form dependency tree
TEST_CASE( "ureact::events<E> (smart pointer semantics)" )
{
    ureact::context ctx;

    ureact::event_source<int> src = ureact::make_source<int>( ctx );

    const auto is_even = []( auto i ) { return i % 2 == 0; };
    const auto is_odd = []( auto i ) { return i % 2 == 1; };

    auto filtered = ureact::filter( src, is_even );

    auto result_even = ureact::collect<std::vector>( filtered );

    //      src         //
    //       |          //
    // filter(is_even)  //
    //       |          //
    //  result_even     //

    // reassigning of 'filtered' doesn't affect result_even, because it depends not on
    // 'filtered' itself, but on reactive node it pointed before
    filtered = ureact::filter( src, is_odd );

    auto result_odd = ureact::collect<std::vector>( filtered );

    //               src                //
    //            /       \             //
    // filter(is_even)   filter(is_odd) //
    //       |                 |        //
    //  result_even        result_odd   //

    std::vector<int> v{ 1, 2, 3, 4, 5, 6 };
    std::copy( v.begin(), v.end(), src.begin() );

    CHECK( result_even.get() == std::vector{ 2, 4, 6 } );
    CHECK( result_odd.get() == std::vector{ 1, 3, 5 } );
}
