#include "copy_stats.hpp"
#include "doctest_extra.h"
#include "ureact/ureact.hpp"

TEST_CASE( "CopyStatsForSignalCalculations" )
{
    ureact::context ctx;

    copy_stats stats;

    auto a = ureact::make_var( ctx, copy_counter{ 1, &stats } );
    auto b = ureact::make_var( ctx, copy_counter{ 10, &stats } );
    auto c = ureact::make_var( ctx, copy_counter{ 100, &stats } );
    auto d = ureact::make_var( ctx, copy_counter{ 1000, &stats } );

    // 4x move to m_value
    // 4x copy to m_new_value (can't be uninitialized for references)
    CHECK( stats.copy_count == 4 );
    CHECK( stats.move_count == 4 );

    auto x = a + b + c + d;

    CHECK( stats.copy_count == 4 );
    CHECK( stats.move_count == 7 );
    CHECK( x.get().v == 1111 );

    a <<= copy_counter{ 2, &stats };

    CHECK( stats.copy_count == 4 );
    CHECK( stats.move_count == 10 );
    CHECK( x.get().v == 1112 );
}
