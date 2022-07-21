#include <nanobench.h>

#include "ureact/ureact.hpp"

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
    auto d = lift( a,
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

    auto b1 = lift( with( a, a ), std::plus<>() );
    auto b2 = lift( with( a, a ), std::plus<>() );
    auto b3 = lift( with( a, a ), std::plus<>() );
    auto b4 = lift( with( a, a ), std::plus<>() );

    auto c1 = lift( with( b1, b2 ), std::multiplies<>() );
    auto c2 = lift( with( b3, b4 ), std::multiplies<>() );

    auto d = lift( with( c1, c2 ), std::plus<>() );

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
