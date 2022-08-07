#include <algorithm>

#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// squaring integer events
TEST_CASE( "Transform" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> squared;
    const auto square = []( auto i ) { return i * i; };

    SUBCASE( "Functional syntax" )
    {
        squared = ureact::transform( src, square );
    }
    SUBCASE( "Piped syntax" )
    {
        squared = src | ureact::transform( square );
    }

    const auto result = ureact::collect<std::vector>( squared );

    for( int i = 0; i < 5; ++i )
        src << i;

    const std::vector<int> expected = { 0, 1, 4, 9, 16 };
    CHECK( result.get() == expected );
}

// clamp integer events with limit which values are in range which is presented in the form
// of signals
TEST_CASE( "TransformSynced" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto limit_min = ureact::make_var( ctx, 4 );
    auto limit_max = ureact::make_var( ctx, 7 );

    ureact::events<int> clamped;
    const auto clamp = []( auto i, int min, int max ) { return std::clamp( i, min, max ); };

    SUBCASE( "Functional syntax" )
    {
        clamped = ureact::transform( src, with( limit_min, limit_max ), clamp );
    }
    SUBCASE( "Piped syntax" )
    {
        clamped = src | ureact::transform( with( limit_min, limit_max ), clamp );
    }

    const auto result = ureact::collect<std::vector>( clamped );

    for( int i : { -1, 4, 10, 0, 5, 2 } )
        src << i;

    // change limits and pass the same values second time
    do_transaction( ctx, [&]() {
        for( int i : { -1, 4, 10, 0, 5, 2 } )
            src << i;

        limit_min <<= 1;
        limit_max <<= 3;
    } );

    // we expect only numbers in [limit_min, limit_max] range passed our filter
    // synced filtering performed only after new limit values are calculated
    const std::vector<int> expected
        = { /*first range*/ 4, 4, 7, 4, 5, 4, /*second range*/ 1, 3, 3, 1, 3, 2 };
    CHECK( result.get() == expected );
}
