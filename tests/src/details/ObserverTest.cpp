#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

namespace
{
using namespace ureact;

} // namespace

TEST_SUITE( "OperationsTest" )
{
    TEST_CASE( "detach" )
    {
        context ctx;

        auto a1 = make_var( ctx, 1 );
        auto a2 = make_var( ctx, 1 );

        auto result = make_signal( with( a1, a2 ), []( int a, int b ) { return a + b; } );

        int observeCount1 = 0;
        int observeCount2 = 0;
        int observeCount3 = 0;

        int phase;

        auto obs1 = observe( result, [&]( int v ) {
            observeCount1++;

            if( phase == 0 )
                CHECK_EQ( v, 3 );
            else if( phase == 1 )
                CHECK_EQ( v, 4 );
            else
                CHECK( false );
        } );

        auto obs2 = observe( result, [&]( int v ) {
            observeCount2++;

            if( phase == 0 )
                CHECK_EQ( v, 3 );
            else if( phase == 1 )
                CHECK_EQ( v, 4 );
            else
                CHECK( false );
        } );

        auto obs3 = observe( result, [&]( int v ) {
            observeCount3++;

            if( phase == 0 )
                CHECK_EQ( v, 3 );
            else if( phase == 1 )
                CHECK_EQ( v, 4 );
            else
                CHECK( false );
        } );

        phase = 0;
        a1 <<= 2;
        CHECK_EQ( observeCount1, 1 );
        CHECK_EQ( observeCount2, 1 );
        CHECK_EQ( observeCount3, 1 );

        phase = 1;
        obs1.detach();
        a1 <<= 3;
        CHECK_EQ( observeCount1, 1 );
        CHECK_EQ( observeCount2, 2 );
        CHECK_EQ( observeCount3, 2 );

        phase = 2;
        obs2.detach();
        obs3.detach();
        a1 <<= 4;
        CHECK_EQ( observeCount1, 1 );
        CHECK_EQ( observeCount2, 2 );
        CHECK_EQ( observeCount3, 2 );
    }

    TEST_CASE( "ScopedObserverTest" )
    {
        std::vector<int> results;

        context ctx;

        auto in = make_var( ctx, 1 );

        {
            scoped_observer obs = observe( in, [&]( int v ) { results.push_back( v ); } );

            in <<= 2;
        }

        in <<= 3;

        CHECK_EQ( results.size(), 1 );
        CHECK_EQ( results[0], 2 );
    }

    TEST_CASE( "SyncedObserveTest" )
    {
        context ctx;

        auto in1 = make_var( ctx, 1 );
        auto in2 = make_var( ctx, 1 );

        auto sum = make_signal( with( in1, in2 ), []( int a, int b ) { return a + b; } );
        auto prod = make_signal( with( in1, in2 ), []( int a, int b ) { return a * b; } );
        auto diff = make_signal( with( in1, in2 ), []( int a, int b ) { return a - b; } );

        auto src1 = make_source( ctx );
        auto src2 = make_source<int>( ctx );

        observe( src1, with( sum, prod, diff ), []( token, int sum, int prod, int diff ) {
            CHECK_EQ( sum, 33 );
            CHECK_EQ( prod, 242 );
            CHECK_EQ( diff, 11 );
        } );

        observe( src2, with( sum, prod, diff ), []( int e, int sum, int prod, int diff ) {
            CHECK_EQ( e, 42 );
            CHECK_EQ( sum, 33 );
            CHECK_EQ( prod, 242 );
            CHECK_EQ( diff, 11 );
        } );

        in1 <<= 22;
        in2 <<= 11;

        src1.emit();
        src2.emit( 42 );
    }

    TEST_CASE( "DetachThisObserver1" )
    {
        context ctx;

        auto src = make_source( ctx );

        int count = 0;

        observe( src, [&]( token ) -> observer_action {
            ++count;
            if( count == 1 )
            {
                return observer_action::next;
            }
            return observer_action::stop_and_detach;
        } );

        src.emit();
        src.emit();
        src.emit();
        src.emit();

        CHECK_EQ( count, 2 );
    }

    TEST_CASE( "DetachThisObserver2" )
    {
        context ctx;

        auto in1 = make_var( ctx, 1 );
        auto in2 = make_var( ctx, 1 );

        auto sum = make_signal( with( in1, in2 ), []( int a, int b ) { return a + b; } );
        auto prod = make_signal( with( in1, in2 ), []( int a, int b ) { return a * b; } );
        auto diff = make_signal( with( in1, in2 ), []( int a, int b ) { return a - b; } );

        auto src = make_source( ctx );

        int count = 0;

        observe( src,
            with( sum, prod, diff ),
            [&]( token, int sum, int prod, int diff ) -> observer_action {
                ++count;
                if( count == 1 )
                {
                    return observer_action::next;
                }
                return observer_action::stop_and_detach;
            } );

        in1 <<= 22;
        in2 <<= 11;

        src.emit();
        src.emit();
        src.emit();
        src.emit();

        CHECK_EQ( count, 2 );
    }

    TEST_CASE( "Detaching observers using return value" )
    {
        context ctx;

        auto x = make_var( ctx, 0 );

        std::vector<int> x_values;

        // Functor used for observer can optionally return value
        // Using this value observer can be optionally self detached
        auto obs = observe( x, [&]( const int v ) {
            if( v < 0 )
            {
                return observer_action::stop_and_detach;
            }
            else
            {
                x_values.push_back( v );
                return observer_action::next;
            }
        } );

        x <<= 1;
        x <<= 2;
        x <<= 3;
        x <<= -1;
        x <<= 4;

        CHECK( x_values == std::vector<int>{ 1, 2, 3 } );
    }
}
