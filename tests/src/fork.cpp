//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/fork.hpp"

#include "doctest_extra.h"
#include "ureact/collect.hpp"
#include "ureact/observe.hpp"
#include "ureact/sink.hpp"
#include "ureact/transform.hpp"

TEST_CASE( "Fork" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    static const auto negate = []( auto i ) { return -i; };
    static const auto x2 = []( auto i ) { return i * 2; };
    static const auto square = []( auto i ) { return i * i; };

    ureact::events<int> dst1;
    ureact::events<int> dst2;
    ureact::events<int> dst;

    // makes void lambda to check if fork supports it
    auto make_transform_to = []( auto func, ureact::events<int>& where ) {
        return [func, &where]( const auto& src ) { //
            std::ignore =                          // TODO: remove it once nodiscard done right
                src | ureact::transform( func ) | ureact::sink( where );
        };
    };

    dst =                                                       //
        src                                                     //
        | ureact::fork(                                         //
            ureact::transform( negate ) | ureact::sink( dst1 ), //
            make_transform_to( x2, dst2 )                       //
            )
        | ureact::transform( square );

    const auto negate_result = ureact::collect<std::vector>( dst1 );
    const auto x2_result = ureact::collect<std::vector>( dst2 );
    const auto square_result = ureact::collect<std::vector>( dst );

    for( int i = 1; i < 4; ++i )
        src << i;

    CHECK( negate_result.get() == std::vector<int>{ -1, -2, -3 } );
    CHECK( x2_result.get() == std::vector<int>{ 2, 4, 6 } );
    CHECK( square_result.get() == std::vector<int>{ 1, 4, 9 } );
}
