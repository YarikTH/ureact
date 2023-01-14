//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/scoped_observer.hpp"

#include "doctest_extra.h"
#include "ureact/observe.hpp"

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
