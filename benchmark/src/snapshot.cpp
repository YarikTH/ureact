#include <sstream>

#include <nanobench.h>

#include "ureact/ureact.hpp"

namespace
{

struct ScenarioSnapshot
{
    explicit ScenarioSnapshot( std::ostream& os )
        : os( os )
    {}

    std::ostream& os;

    using S = int;
    using E = int;

    static const char* name()
    {
        return "Snapshot";
    }

    static S event_value( int a )
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

template <class Scenario, class S = typename Scenario::S, class E = typename Scenario::E>
void perform_test( Scenario& scenario,
    const char* name,
    ankerl::nanobench::Bench& bench,
    ureact::context& ctx,
    const ureact::event_source<E>& trigger,
    const ureact::var_signal<S>& target,
    const ureact::signal<S>& out )
{
    int i = 0;

    int changes = 0;
    observe( out, [&]( const S& ) { ++changes; } );

    bench.run( name, [&] {
        // ensure changing the target do not change the out
        target <<= i * 10;
        assert( out.get() != target.get() );

        // ensure emitting the trigger changes the out
        target <<= i;
        trigger.emit( E{} );
        assert( out.get() == i );

        // additionally trigger to ensure it does not add additional changes
        for( int j = 0; j < 10; ++j )
            trigger.emit( E{} );

        bench.doNotOptimizeAway( out.get() );
        ++i;
    } );

    scenario.check_changes( name, i, changes );
}

// Standard snapshot algorithm
template <class Scenario, class S = typename Scenario::S, class E = typename Scenario::E>
void snapshot_standard( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto trigger = ureact::make_event_source<E>( ctx );
    auto target = ureact::make_var( ctx, S{} );

    auto out = ureact::snapshot( trigger, target );

    perform_test( scenario, "snapshot", bench, ctx, trigger, target, out );
}

// Standard snapshot algorithm
template <class Scenario, class S = typename Scenario::S, class E = typename Scenario::E>
void snapshot_standard_piped( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto trigger = ureact::make_event_source<E>( ctx );
    auto target = ureact::make_var( ctx, S{} );

    auto out = trigger | ureact::snapshot( target );

    perform_test( scenario, "snapshot piped", bench, ctx, trigger, target, out );
}

// Snapshot algorithm via fold with event_range
template <class Scenario, class S = typename Scenario::S, class E = typename Scenario::E>
void snapshot_fold_ranged( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto trigger = ureact::make_event_source<E>( ctx );
    auto target = ureact::make_var( ctx, S{} );

    auto out = fold( trigger,
        target.get(),
        with( target ),
        []( ureact::event_range<E> range, const S&, const S& value ) { return value; } );

    perform_test( scenario, "fold ranged", bench, ctx, trigger, target, out );
}

// Snapshot algorithm via fold
template <class Scenario, class S = typename Scenario::S, class E = typename Scenario::E>
void snapshot_fold( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto trigger = ureact::make_event_source<E>( ctx );
    auto target = ureact::make_var( ctx, S{} );

    auto out = fold( trigger, target.get(), with( target ), []( E, const S&, const S& value ) {
        return value;
    } );

    perform_test( scenario, "fold", bench, ctx, trigger, target, out );
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
    snapshot_standard( scenario, b );
    snapshot_standard_piped( scenario, b );
    snapshot_fold_ranged( scenario, b );
    snapshot_fold( scenario, b );

    *b.output() << errors.str() << '\n';
}

} // namespace

int main()
{
    run_scenario<ScenarioSnapshot>();
}
