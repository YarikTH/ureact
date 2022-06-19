#include <algorithm>
#include <queue>
#include <string>

#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

namespace
{
using namespace ureact;

} // namespace

TEST_SUITE( "EventStreamTest" )
{
    TEST_CASE( "EventSources" )
    {
        context ctx;

        auto es1 = make_event_source<int>( ctx );
        auto es2 = make_event_source<int>( ctx );

        std::queue<int> results1;
        std::queue<int> results2;

        observe( es1, [&]( int v ) { results1.push( v ); } );

        observe( es2, [&]( int v ) { results2.push( v ); } );

        es1 << 10 << 20 << 30;
        es2 << 40 << 50 << 60;

        // 1
        CHECK_FALSE( results1.empty() );
        CHECK_EQ( results1.front(), 10 );
        results1.pop();

        CHECK_FALSE( results1.empty() );
        CHECK_EQ( results1.front(), 20 );
        results1.pop();

        CHECK_FALSE( results1.empty() );
        CHECK_EQ( results1.front(), 30 );
        results1.pop();

        CHECK( results1.empty() );

        // 2
        CHECK_FALSE( results2.empty() );
        CHECK_EQ( results2.front(), 40 );
        results2.pop();

        CHECK_FALSE( results2.empty() );
        CHECK_EQ( results2.front(), 50 );
        results2.pop();

        CHECK_FALSE( results2.empty() );
        CHECK_EQ( results2.front(), 60 );
        results2.pop();

        CHECK( results2.empty() );
    }
}
