#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// filters that take first N elements or skip first N elements
TEST_CASE( "TakeOrDrop" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::events<int> first_n;
    ureact::events<int> without_first_n;

    SUBCASE( "Functional syntax" )
    {
        first_n = ureact::take( src, 5 );
        without_first_n = ureact::drop( src, 5 );
    }
    SUBCASE( "Piped syntax" )
    {
        first_n = src | ureact::take( 5 );
        without_first_n = src | ureact::drop( 5 );
    }

    const auto result_first_n = ureact::collect<std::vector>( first_n );
    const auto result_without_first_n = ureact::collect<std::vector>( without_first_n );

    // pass integers as events
    for( int i : { 0, 1, 2, 3, 4, 5, -1, 6, 7, 8, 9 } )
        src << i;

    // if we concatenate results of take(N) and drop(N) we receive original set of events
    const std::vector<int> expected_first_n = //
        { 0, 1, 2, 3, 4 };
    const std::vector<int> expected_without_first_n = //
        /*          */ { 5, -1, 6, 7, 8, 9 };
    CHECK( result_first_n.get() == expected_first_n );
    CHECK( result_without_first_n.get() == expected_without_first_n );
}
