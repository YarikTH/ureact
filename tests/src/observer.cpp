#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// default constructive
static_assert( std::is_default_constructible_v<ureact::observer> );

// nothrow movable only
static_assert( !std::is_copy_constructible_v<ureact::observer> );
static_assert( !std::is_copy_assignable_v<ureact::observer> );
static_assert( std::is_move_constructible_v<ureact::observer> );
static_assert( std::is_move_assignable_v<ureact::observer> );
static_assert( std::is_nothrow_move_constructible_v<ureact::observer> );
static_assert( std::is_nothrow_move_assignable_v<ureact::observer> );


// scoped_observer is not intended to be default constructive, but move constructive from observer
static_assert( !std::is_default_constructible_v<ureact::scoped_observer> );
static_assert( std::is_constructible_v<ureact::scoped_observer, ureact::observer&&> );
static_assert( !std::is_constructible_v<ureact::scoped_observer, const ureact::observer&> );

// nothrow movable only
static_assert( !std::is_copy_constructible_v<ureact::scoped_observer> );
static_assert( !std::is_copy_assignable_v<ureact::scoped_observer> );
static_assert( std::is_move_constructible_v<ureact::scoped_observer> );
static_assert( std::is_move_assignable_v<ureact::scoped_observer> );
static_assert( std::is_nothrow_move_constructible_v<ureact::scoped_observer> );
static_assert( std::is_nothrow_move_assignable_v<ureact::scoped_observer> );

// TODO: test .detach()


// Functor used for observer can optionally return value
// Using this value observer can be optionally self detached
TEST_CASE( "SelfObserverDetach" )
{
    ureact::context ctx;

    std::vector<int> results;
    auto observe_handler = [&]( int v ) {
        if( v < 0 )
        {
            return ureact::observer_action::stop_and_detach;
        }
        else
        {
            results.push_back( v );
            return ureact::observer_action::next;
        }
    };

    std::vector input_values{ 1, 2, 3, -1, 4 };

    SUBCASE( "signal" )
    {
        auto src = ureact::make_var<int>( ctx, -1 );

        ureact::observer obs = observe( src, observe_handler );

        for( int i : input_values )
            src <<= i;
    }
    SUBCASE( "source" )
    {
        auto src = ureact::make_source<int>( ctx );

        ureact::observer obs = observe( src, observe_handler );

        for( int i : input_values )
            src << i;
    }

    CHECK( results == std::vector{ 1, 2, 3 } );
}

// During its lifetime, the observer handle of an observer guarantees that the
// observed subject will not be destroyed and allows explicit detach.
// But even after the observer handle is destroyed, the subject may continue to exist
// and so will the observer.
//
// scoped_observer has similar semantics to a scoped lock.
// When it's destroyed, it detaches and destroys the observer.
TEST_CASE( "ScopedObserver" )
{
    ureact::context ctx;

    auto src = make_source( ctx );

    size_t observation_count = 0;
    auto observe_handler = [&]( auto&& ) { ++observation_count; };

    CHECK( observation_count == 0 );

    SUBCASE( "ScopedObserver" )
    {
        ureact::scoped_observer obs = observe( src, observe_handler );

        src.emit();
    }
    SUBCASE( "ObserverDetach" )
    {
        ureact::observer obs = observe( src, observe_handler );

        src.emit();

        obs.detach(); // scoped_observer does the same in the descriptor
    }

    CHECK( observation_count == 1 );

    src.emit(); // observer is detached, observation is not performed

    CHECK( observation_count == 1 );
}
