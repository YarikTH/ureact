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

    using E = int;

    static const char* name()
    {
        return "Snapshot";
    }

    static E event_value( int a )
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

template <class Scenario, class E = typename Scenario::E>
void perform_test( Scenario& scenario,
    const char* name,
    ankerl::nanobench::Bench& bench,
    ureact::context& ctx,
    const ureact::event_source<ureact::token>& trigger,
    const ureact::var_signal<E>& target,
    const ureact::signal<E>& out )
{
    int i = 0;

    int changes = 0;
    observe( out, [&]( const E& ) { ++changes; } );

    bench.run( name, [&] {
        // ensure changing the target do not changes the out
        target <<= i * 10;
        assert( out.get() != target.get() );

        // ensure emitting the trigger changes the out
        target <<= i;
        trigger.emit();
        assert( out.get() == i );

        // additionally trigger to ensure it does not add additional changes
        for( int j = 0; j < 10; ++j )
            trigger.emit();

        bench.doNotOptimizeAway( out.get() );
        ++i;
    } );

    scenario.check_changes( name, i, changes );
}

// Standard snapshot algorithm
template <class Scenario, class E = typename Scenario::E>
void snapshot_standard( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto trigger = ureact::make_event_source( ctx );
    auto target = ureact::make_var( ctx, E{} );

    auto out = ureact::snapshot( trigger, target );

    perform_test( scenario, "snapshot", bench, ctx, trigger, target, out );
}

// Snapshot algorithm via fold with event_range
template <class Scenario, class E = typename Scenario::E>
void snapshot_fold_ranged( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto trigger = ureact::make_event_source( ctx );
    auto target = ureact::make_var( ctx, E{} );

    auto out = fold( trigger, target.get(), with( target ), []( ureact::event_range<> range, const E&, const E& value ) {
        return value;
    } );

    perform_test( scenario, "fold ranged", bench, ctx, trigger, target, out );
}

// Snapshot algorithm via fold
template <class Scenario, class E = typename Scenario::E>
void snapshot_fold( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto trigger = ureact::make_event_source( ctx );
    auto target = ureact::make_var( ctx, E{} );

    auto out = fold( trigger, target.get(), with( target ), []( ureact::token, const E&, const E& value ) {
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
    snapshot_fold_ranged( scenario, b );
    snapshot_fold( scenario, b );

    *b.output() << errors.str() << '\n';
}

} // namespace

int main()
{
    run_scenario<ScenarioSnapshot>();
}
