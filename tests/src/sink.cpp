//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/sink.hpp"

#include "doctest_extra.h"
#include "ureact/collect.hpp"
#include "ureact/filter.hpp"
#include "ureact/lift.hpp"
#include "ureact/observe.hpp"
#include "ureact/transform.hpp"

TEST_CASE( "SinkSignal" )
{
    ureact::context ctx;

    ureact::var_signal<int> src = make_var( ctx, 1 );
    ureact::signal<int> squared;
    ureact::signal<int> minus_squared;
    static const auto square = []( auto i ) { return i * i; };

    std::ignore =                                  // TODO: remove it once nodiscard done right
        src                                        //
        | ureact::lift( square ) | sink( squared ) //
        | ureact::lift( std::negate<>{} ) | sink( minus_squared );

    CHECK( squared.get() == 1 );
    CHECK( minus_squared.get() == -1 );

    src <<= -2;

    CHECK( squared.get() == 4 );
    CHECK( minus_squared.get() == -4 );
}

TEST_CASE( "SinkEvents" )
{
    ureact::context ctx;

    ureact::event_source src = ureact::make_source<int>( ctx );
    ureact::events<int> even;
    ureact::events<int> even_squared;
    static auto is_even = []( int i ) { return 0 == i % 2; };
    static const auto square = []( auto i ) { return i * i; };

    std::ignore =                                  // TODO: remove it once nodiscard done right
        src                                        //
        | ureact::filter( is_even ) | sink( even ) //
        | ureact::transform( square ) | sink( even_squared );

    const auto even_result = ureact::collect<std::vector>( even );
    const auto even_squared_result = ureact::collect<std::vector>( even_squared );

    for( int i = 0; i < 6; ++i )
        src << i;

    CHECK( even_result.get() == std::vector<int>{ 0, 2, 4 } );
    CHECK( even_squared_result.get() == std::vector<int>{ 0, 4, 16 } );
}

TEST_CASE( "SinkObservers" )
{
    ureact::context ctx;

    ureact::event_source<int> src = ureact::make_source<int>( ctx );
    ureact::observer obs;
    std::vector<int> result;
    static auto collector = [&]( int i ) { result.push_back( i ); };

    std::ignore = // TODO: remove it once nodiscard done right
        src | ureact::observe( collector ) | sink( obs );

    for( int i = 0; i < 3; ++i )
        src << i;

    CHECK( result == std::vector<int>{ 0, 1, 2 } );
}
