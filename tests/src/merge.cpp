#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// merge 3 event sources into 1
TEST_CASE( "Merge" )
{
    ureact::context ctx;

    auto src1 = ureact::make_source<int>( ctx );
    auto src2 = ureact::make_source<int>( ctx );
    auto src3 = ureact::make_source<int>( ctx );

    ureact::events<int> src = merge( src1, src2, src3 );

    const auto result = ureact::collect<std::vector>( src );

    src1 << 1 << 5;
    src2 << -1;
    src3 << 9;
    src2 << 0;

    const std::vector<int> expected = { 1, 5, -1, 9, 0 };
    CHECK( result.get() == expected );
}
