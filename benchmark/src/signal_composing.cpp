//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <nanobench.h>

#include "ureact/lift.hpp"

namespace
{

void perform_test( const char* name,
    ankerl::nanobench::Bench& bench,
    const ureact::var_signal<int>& in,
    const ureact::signal<int>& out )
{
    assert( out.get() == 8 );

    int i = 0;

    bench.run( name, [&] {
        in <<= i;
        assert( out.get() == 8 * i * i );
        bench.doNotOptimizeAway( out.get() );
        ++i;
    } );
}

// Take signal constructed with lambda expression as a baseline
void signal_functions_baseline( ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto a = make_var( ctx, 1 );
    auto d = ureact::lift( a,
        []( int a ) { //
            return ( ( a + a ) * ( a + a ) ) + ( ( a + a ) * ( a + a ) );
        } );

    perform_test( "baseline", bench, a, d );
}

// Signal constructed from the expression. Intermediate signals should be optimized by ureact
void signal_functions_expression( ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto a = make_var( ctx, 1 );
    auto d = ( ( a + a ) * ( a + a ) ) + ( ( a + a ) * ( a + a ) );

    perform_test( "expression", bench, a, d );
}

// Make a separate signal for each calculation step and store them into variable
// to make them L-value, so they can't be optimized by ureact
void signal_functions_separate( ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto a = make_var( ctx, 1 );

    auto b1 = a + a;
    auto b2 = a + a;
    auto b3 = a + a;
    auto b4 = a + a;

    auto c1 = b1 * b2;
    auto c2 = b3 * b4;

    auto d = c1 + c2;

    perform_test( "separate", bench, a, d );
}

} // namespace

int main()
{
    ankerl::nanobench::Bench b;
    b.title( "Signal composing comparison" );
    b.warmup( 100 );
    b.relative( true );
    b.performanceCounters( false );

    signal_functions_baseline( b );
    signal_functions_expression( b );
    signal_functions_separate( b );
}
