#include <cassert>
#include <sstream>

#include <nanobench.h>

#include "ureact/ureact.hpp"

namespace
{

const auto is_even = []( auto i ) { return i % 2 == 0; };

struct ScenarioFilter
{
    explicit ScenarioFilter( std::ostream& os )
        : os( os )
    {}

    std::ostream& os;

    static constexpr size_t N = 10000;

    using E = int;

    static const char* name()
    {
        return "Filter";
    }

    //    void check_changes( const char* name, int loops, int changes )
    //    {
    //        const int expected = loops * 43;
    //        if( std::abs( expected - changes ) > 2 )
    //            os << "error: \"" << name << "\" algorithm made unexpected change notifications. "
    //               << expected << " are expected, but " << changes << " got.\n";
    //    }
};

template <class Scenario, class E = typename Scenario::E>
void perform_test( Scenario& scenario,
    const char* name,
    ankerl::nanobench::Bench& bench,
    ureact::context& ctx,
    const ureact::event_source<E>& source,
    const ureact::events<E>& out )
{
    int i = 0;

    std::vector<E> values;
    values.resize( Scenario::N );
    observe( out, [&]( const E& e ) { values.push_back( e ); } );

    bench.run( name, [&] {
        values.clear();

        for( int j = 0; j < Scenario::N; ++j )
            source.emit( j );

        assert( values.size() == Scenario::N / 2 );

        values.clear();

        ctx.template do_transaction( [&]() {
            for( int j = 0; j < Scenario::N; ++j )
                source.emit( j );
        } );

        assert( values.size() == Scenario::N / 2 );

        ++i;
    } );
}

// Standard filter algorithm
template <class Scenario, class E = typename Scenario::E>
void filter_standard( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto source = ureact::make_event_source<E>( ctx );

    auto out = ureact::filter( source, is_even );

    perform_test( scenario, "filter", bench, ctx, source, out );
}

// Filter algorithm via process algorithm
template <class Scenario, class E = typename Scenario::E>
void filter_process( Scenario& scenario, ankerl::nanobench::Bench& bench )
{
    ureact::context ctx;
    auto source = ureact::make_event_source<E>( ctx );

    auto out = ureact::process<E>(
        source, []( ureact::event_range<E> range, ureact::event_emitter<E> out ) {
            for( const auto& e : range )
                if( is_even( e ) )
                    out.emit( e );
        } );

    perform_test( scenario, "process", bench, ctx, source, out );
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
    filter_standard( scenario, b );
    filter_process( scenario, b );

    *b.output() << errors.str() << '\n';
}

} // namespace

int main()
{
    run_scenario<ScenarioFilter>();
}
