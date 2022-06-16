#include <cassert>
#include <sstream>

#include <nanobench.h>

#include "ureact/ureact.hpp"

namespace
{

struct ScenarioPulse
{
    explicit ScenarioPulse( std::ostream& os )
        : os( os )
    {}

    std::ostream& os;

    using S = int;
    using E = int;

    static const char* name()
    {
        return "Pulse";
    }

    static S event_value( int a )
    {
        return a;
    }

    void check_changes( const char* name, int loops, int changes )
    {
        const int expected = loops * 43;
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
    const ureact::events<S>& out )
{
    int i = 0;

    int changes = 0;
    std::vector<S> values;
    values.resize( 100 );
    observe( out, [&]( const S& ) { ++changes; } );
    observe( out, [&]( const S& s ) { values.push_back( s ); } );

    bench.run( name, [&] {
        values.clear();

        target <<= i;
        trigger.emit( E{} );

        assert( values.size() == 1 );
        assert( values[0] == i );

        ctx.do_transaction( [&]() {
            target <<= i * 3;
            trigger.emit( E{} );
            trigger.emit( E{} );
        } );

        assert( values.size() == 3 );
        assert( values[0] == i );
        assert( values[1] == i * 3 );
        assert( values[2] == i * 3 );

        // emit lots of events in one transaction
        ctx.do_transaction( [&]() {
            target <<= i * 5;
            for( int j = 0; j < 40; ++j )
                trigger.emit( E{} );
        } );

        assert( values.size() == 43 );

        ++i;
    } );

    scenario.check_changes( name, i, changes );
}

// Standard pulse algorithm
template <class Scenario, class S = typename Scenario::S, class E = typename Scenario::E>
void pulse_standard( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto trigger = ureact::make_event_source<E>( ctx );
    auto target = ureact::make_var( ctx, S{} );

    auto out = ureact::pulse( trigger, target );

    perform_test( scenario, "pulse", bench, ctx, trigger, target, out );
}

// Standard pulse algorithm piped
template <class Scenario, class S = typename Scenario::S, class E = typename Scenario::E>
void pulse_standard_piped( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto trigger = ureact::make_event_source<E>( ctx );
    auto target = ureact::make_var( ctx, S{} );

    auto out = trigger | ureact::pulse( target );

    perform_test( scenario, "pulse piped", bench, ctx, trigger, target, out );
}

// Pulse algorithm via synced process algorithm
template <class Scenario, class S = typename Scenario::S, class E = typename Scenario::E>
void pulse_process( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto trigger = ureact::make_event_source<E>( ctx );
    auto target = ureact::make_var( ctx, S{} );

    auto out = ureact::process<S>( trigger,
        with( target ),
        []( ureact::event_range<E> range, ureact::event_emitter<S> out, const S& target_value ) {
            for( size_t i = 0, ie = range.size(); i < ie; ++i )
                out << target_value;
        } );

    perform_test( scenario, "process", bench, ctx, trigger, target, out );
}

// Pulse algorithm via synced process algorithm
template <class Scenario, class S = typename Scenario::S, class E = typename Scenario::E>
void pulse_process_2( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto trigger = ureact::make_event_source<E>( ctx );
    auto target = ureact::make_var( ctx, S{} );

    auto out = ureact::process<S>( trigger,
        with( target ),
        []( ureact::event_range<E> range, ureact::event_emitter<S> out, const S& target_value ) {
            for( size_t i = 0, ie = range.size(); i < ie; ++i )
                out.emit( target_value );
        } );

    perform_test( scenario, "process 2", bench, ctx, trigger, target, out );
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
    pulse_standard( scenario, b );
    pulse_standard_piped( scenario, b );
    pulse_process( scenario, b );
    pulse_process_2( scenario, b );

    *b.output() << errors.str() << '\n';
}

} // namespace

int main()
{
    run_scenario<ScenarioPulse>();
}
