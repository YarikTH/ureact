//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <nanobench.h>

#include "catch2_extra.hpp"
#include "ureact/adaptor/observe.hpp"
#include "ureact/adaptor/transform.hpp"
#include "ureact/adaptor/transform2.hpp"
#include "ureact/adaptor/transform3.hpp"
#include "ureact/adaptor/transform4.hpp"
#include "ureact/adaptor/transform5.hpp"
#include "ureact/transaction.hpp"

namespace
{

void perform_test( const char* name,
    ankerl::nanobench::Bench& bench,
    const ureact::event_source<int>& in,
    const ureact::events<int>& out )
{
    int i = 0;
    std::vector<int> result{};
    auto obs = ureact::observe( out, std::back_inserter( result ) );

    bench.run( name, [&] {
        result.clear();
        for( size_t j = 0; j < 100; ++j )
            in << i;
        {
            ureact::transaction _{ in.get_context() };
            for( size_t j = 0; j < 100; ++j )
                in << i;
        }
        bench.doNotOptimizeAway( result );
        ++i;
    } );
}

#define CONCAT_IMPL( LHS, RHS ) LHS##RHS
#define CONCAT( LHS, RHS ) CONCAT_IMPL( LHS, RHS )

#define MAKE_TRANSFORM_TEST( _NAME_ )                                                              \
    void CONCAT( test_, _NAME_ )( ankerl::nanobench::Bench & bench )                               \
    {                                                                                              \
        ureact::context ctx;                                                                       \
        auto src = ureact::make_source<int>( ctx );                                                \
        ureact::events out = src;                                                                  \
        for( size_t i = 0; i < 100; ++i )                                                          \
            out = ureact::_NAME_( out, std::negate<>{} );                                          \
        perform_test( #_NAME_, bench, src, out );                                                  \
    }

MAKE_TRANSFORM_TEST( transform )
MAKE_TRANSFORM_TEST( transform2 )
MAKE_TRANSFORM_TEST( transform3 )
MAKE_TRANSFORM_TEST( transform4 )
MAKE_TRANSFORM_TEST( transform5 )

} // namespace

TEST_CASE( "Benchmark transform variants" )
{
    ankerl::nanobench::Bench bench;
    bench.title( "Transform variants" );
    bench.warmup( 100 );
    bench.relative( true );
    bench.performanceCounters( false );
    //bench.minEpochIterations( 50 );

    test_transform( bench );
    test_transform2( bench );
    test_transform3( bench );
    test_transform4( bench );
    test_transform5( bench );
}
