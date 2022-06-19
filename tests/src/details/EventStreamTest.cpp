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

    TEST_CASE( "EventMerge1" )
    {
        context ctx;

        auto a1 = make_event_source<int>( ctx );
        auto a2 = make_event_source<int>( ctx );
        auto a3 = make_event_source<int>( ctx );

        events<int> merged;

        SUBCASE( "Function" )
        {
            merged = merge( a1, a2, a3 );
        }

        std::vector<int> results;

        observe( merged, [&]( int v ) { results.push_back( v ); } );

        ctx.do_transaction( [&] {
            a1 << 10;
            a2 << 20;
            a3 << 30;
        } );

        // Order is arbitrary, so we need to sort it before compare
        std::sort( results.begin(), results.end() );
        CHECK( results == std::vector<int>{ 10, 20, 30 } );
    }

    TEST_CASE( "EventMerge2" )
    {
        context ctx;

        auto a1 = make_event_source<std::string>( ctx );
        auto a2 = make_event_source<std::string>( ctx );
        auto a3 = make_event_source<std::string>( ctx );

        auto merged = merge( a1, a2, a3 );

        std::vector<std::string> results;

        observe( merged, [&]( const std::string& s ) { results.push_back( s ); } );

        std::string s1( "one" );
        std::string s2( "two" );
        std::string s3( "three" );

        ctx.do_transaction( [&] {
            a1 << s1;
            a2 << s2;
            a3 << s3;
        } );

        // Order is arbitrary, so we need to sort it before compare
        std::sort( results.begin(), results.end() );
        CHECK( results == std::vector<std::string>{ "one", "three", "two" } );
    }

    TEST_CASE( "EventMerge3" )
    {
        context ctx;

        auto a1 = make_event_source<int>( ctx );
        auto a2 = make_event_source<int>( ctx );

        auto f1 = filter( a1, []( int v ) { return true; } );
        auto f2 = filter( a2, []( int v ) { return true; } );

        auto merged = merge( f1, f2 );

        std::queue<int> results;

        observe( merged, [&]( int s ) { results.push( s ); } );

        a1 << 10;
        a2 << 20;
        a1 << 30;

        CHECK_FALSE( results.empty() );
        CHECK_EQ( results.front(), 10 );
        results.pop();

        CHECK_FALSE( results.empty() );
        CHECK_EQ( results.front(), 20 );
        results.pop();

        CHECK_FALSE( results.empty() );
        CHECK_EQ( results.front(), 30 );
        results.pop();

        CHECK( results.empty() );
    }

    TEST_CASE( "EventProcess" )
    {
        std::vector<float> results;

        context ctx;

        auto in1 = make_event_source<int>( ctx );
        auto in2 = make_event_source<int>( ctx );

        auto merged = merge( in1, in2 );
        int callCount = 0;

        auto processed
            = process<float>( merged, [&]( event_range<int> range, event_emitter<float> out ) {
                  for( const auto& e : range )
                  {
                      *out = 0.1f * static_cast<float>( e );
                      *out = 1.5f * static_cast<float>( e );
                  }

                  callCount++;
              } );

        observe( processed, [&]( float s ) { results.push_back( s ); } );

        ctx.do_transaction( [&] { in1 << 10 << 20; } );

        in2 << 30;

        CHECK( results == std::vector<float>{ 1.0f, 15.0f, 2.0f, 30.0f, 3.0f, 45.0f } );
        CHECK_EQ( callCount, 2 );
    }
}
