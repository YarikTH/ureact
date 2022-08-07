#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// Emits value of target signal when event is received
TEST_CASE( "Pulse" )
{
    ureact::context ctx;

    auto trigger = ureact::make_source( ctx );
    auto target = ureact::make_var<int>( ctx, -1 );
    ureact::events<int> beat;

    SUBCASE( "Functional syntax" )
    {
        beat = ureact::pulse( trigger, target );
    }
    SUBCASE( "Piped syntax" )
    {
        beat = trigger | ureact::pulse( target );
    }

    SUBCASE( "Trigger can be any type" )
    {
        beat = ureact::transform( trigger, []( ureact::unit ) { return 1; } )
             | ureact::pulse( target );
    }

    auto result = ureact::collect<std::vector>( beat );

    target <<= 1;
    for( int i = 0; i < 2; ++i )
        trigger();

    do_transaction( ctx, [&]() {
        target <<= 6;
        for( int i = 0; i < 3; ++i )
            trigger();
    } );

    // we expect first value twice and second value thrice
    CHECK( result.get() == std::vector{ 1, 1, 6, 6, 6 } );
}
