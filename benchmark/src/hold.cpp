#include <sstream>

#include <nanobench.h>

#include "ureact/ureact.hpp"

namespace
{

struct ScenarioBatchNTheSameEvents
{
    explicit ScenarioBatchNTheSameEvents( std::ostream& os )
        : os( os )
    {}

    std::ostream& os;

    using E = int;

    static constexpr size_t N = 10000;

    static const char* name()
    {
        return "BatchNTheSameEvents";
    }

    static E event_value( int a, int b )
    {
        return a;
    }

    void check_changes( const char* name, int loops, int changes )
    {
        const int expected = loops;
        if( std::abs( expected - changes ) > 2 )
            os << "error: \"" << name << "\" algorithm made unexpected change notifications. "
               << expected << " are expected, but " << changes << " got.\n";
    }
};

struct ScenarioBatchNDifferentEvents
{
    explicit ScenarioBatchNDifferentEvents( std::ostream& os )
        : os( os )
    {}

    std::ostream& os;

    using E = int;

    static constexpr size_t N = 10000;

    static const char* name()
    {
        return "BatchNDifferentEvents";
    }

    static E event_value( int a, int b )
    {
        return a + b;
    }

    void check_changes( const char* name, int loops, int changes )
    {
        const int expected = loops;
        if( std::abs( expected - changes ) > 2 )
            os << "error: \"" << name << "\" algorithm made unexpected change notifications. "
               << expected << " are expected, but " << changes << " got.\n";
    }
};

struct ScenarioBatchNTheSameEventsSeveralTimes
{
    explicit ScenarioBatchNTheSameEventsSeveralTimes( std::ostream& os )
        : os( os )
    {}

    std::ostream& os;

    using E = int;

    static constexpr size_t N = 10000;

    static const char* name()
    {
        return "ScenarioBatchNTheSameEventsSeveralTimes";
    }

    static E event_value( int a, int b )
    {
        return a / 5;
    }

    void check_changes( const char* name, int loops, int changes )
    {
        const int expected = loops / 5;
        if( std::abs( expected - changes ) > 10 )
            os << "error: \"" << name << "\" algorithm made unexpected change notifications. About "
               << expected << " are expected, but " << changes << " got.\n";
    }
};

template <class Scenario, class E = typename Scenario::E>
void test_baseline( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    constexpr size_t N = Scenario::N;

    int i = 0;

    bench.run( "baseline", [&] {
        E out;
        for( int j = 0; j <= N; ++j )
            out = Scenario::event_value( i, j );

        assert( out == Scenario::event_value( i, N ) );

        bench.doNotOptimizeAway( out );
        ++i;
    } );
}

template <class Scenario, class E = typename Scenario::E>
void perform_test( Scenario& scenario,
    const char* name,
    ankerl::nanobench::Bench& bench,
    ureact::context& ctx,
    const ureact::event_source<E>& in,
    const ureact::signal<E>& out )
{
    constexpr size_t N = Scenario::N;

    int changes = 0;
    observe( out, [&]( const E& ) { ++changes; } );

    int i = 0;

    bench.run( name, [&] {
        ctx.do_transaction( [&]() {
            for( int j = 0; j <= N; ++j )
                in << Scenario::event_value( i, j );
        } );

        assert( out.get() == Scenario::event_value( i, N ) );

        bench.doNotOptimizeAway( out.get() );
        ++i;
    } );

    scenario.check_changes( name, i, changes );
}

// Original hold algorithm
template <class Scenario, class E = typename Scenario::E>
void hold_original( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto src = ureact::make_event_source<E>( ctx );

    auto out = hold( src, E{} );

    perform_test( scenario, "original", bench, ctx, src, out );
}

// New hold algorithm
template <class Scenario, class E = typename Scenario::E>
void hold_new( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto src = ureact::make_event_source<E>( ctx );

    auto out = hold2( src, E{} );

    perform_test( scenario, "new", bench, ctx, src, out );
}

// Hold algorithm via fold with event_range
template <class Scenario, class E = typename Scenario::E>
void hold_fold_ranged( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto src = ureact::make_event_source<E>( ctx );

    auto out
        = fold( src, E{}, []( ureact::event_range<E> range, E accum ) { return *range.rbegin(); } );

    perform_test( scenario, "fold ranged", bench, ctx, src, out );
}

// Hold algorithm via fold with event_range
template <class Scenario, class E = typename Scenario::E>
void hold_fold_ranged_2( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto src = ureact::make_event_source<E>( ctx );

    auto out = fold(
        src, E{}, []( ureact::event_range<E> range, const E& ) { return *range.rbegin(); } );

    perform_test( scenario, "fold ranged 2", bench, ctx, src, out );
}

// Hold algorithm via fold
template <class Scenario, class E = typename Scenario::E>
void hold_fold( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto src = ureact::make_event_source<E>( ctx );

    auto out = fold( src, E{}, []( E i, const E& ) { return std::move( i ); } );

    perform_test( scenario, "fold", bench, ctx, src, out );
}

// Hold algorithm via fold with event_range
template <class Scenario, class E = typename Scenario::E>
void hold_fold_ranged_by_ref( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto src = ureact::make_event_source<E>( ctx );

    auto out = fold( src, E{}, []( ureact::event_range<E> range, E& accum ) {
        const E& new_value = *range.rbegin();
        if( accum != new_value )
            accum = new_value;
    } );

    perform_test( scenario, "fold ranged by ref", bench, ctx, src, out );
}

// Hold algorithm via fold
template <class Scenario, class E = typename Scenario::E>
void hold_fold_by_ref( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto src = ureact::make_event_source<E>( ctx );

    auto out = fold( src, E{}, []( E i, E& accum ) { accum = i; } );

    perform_test( scenario, "fold by ref", bench, ctx, src, out );
}

template <class Scenario>
void run_scenario()
{
    ankerl::nanobench::Bench b;
    b.title( Scenario::name() );
    b.warmup( 1000 );
    b.relative( true );
    b.performanceCounters( false );

    std::ostringstream errors;
    Scenario scenario{ errors };

    //test_baseline( scenario, b );
    hold_original( scenario, b );
    hold_new( scenario, b );
    hold_fold_ranged( scenario, b );
    hold_fold_ranged_2( scenario, b );
    hold_fold( scenario, b );
    hold_fold_ranged_by_ref( scenario, b );
    hold_fold_by_ref( scenario, b );

    *b.output() << errors.str() << '\n';
}

} // namespace

int main()
{
    run_scenario<ScenarioBatchNTheSameEvents>();
    run_scenario<ScenarioBatchNTheSameEventsSeveralTimes>();
    run_scenario<ScenarioBatchNDifferentEvents>();
}
