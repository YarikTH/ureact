#include <algorithm>
#include <queue>

#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

namespace
{
using namespace ureact;

template <typename T>
struct Incrementer
{
    T operator()( T v, token ) const
    {
        return v + 1;
    }
};

template <typename T>
struct Decrementer
{
    T operator()( T v, token ) const
    {
        return v - 1;
    }
};
} // namespace

TEST_SUITE( "OperationsTest" )
{
    TEST_CASE( "Iterate1" )
    {
        context ctx;

        auto numSrc = make_event_source<int>( ctx );
        auto numFold = fold( numSrc, 0, []( int d, int v ) { return v + d; } );

        for( auto i = 1; i <= 100; i++ )
        {
            numSrc << i;
        }

        CHECK_EQ( numFold(), 5050 );

        auto charSrc = make_event_source<char>( ctx );
        auto strFold = fold(
            charSrc, std::string( "" ), []( const std::string& s, char c ) { return s + c; } );

        charSrc << 'T' << 'e' << 's' << 't';

        CHECK_EQ( strFold(), "Test" );
    }

    TEST_CASE( "Iterate2" )
    {
        context ctx;

        auto numSrc = make_event_source<int>( ctx );
        auto numFold = fold( numSrc, 0, []( int d, int v ) { return v + d; } );

        int c = 0;

        observe( numFold, [&]( int v ) {
            c++;
            CHECK_EQ( v, 5050 );
        } );

        ctx.do_transaction( [&] {
            for( auto i = 1; i <= 100; i++ )
                numSrc << i;
        } );

        CHECK_EQ( numFold(), 5050 );
        CHECK_EQ( c, 1 );
    }

    TEST_CASE( "Iterate3" )
    {
        context ctx;

        auto trigger = make_event_source( ctx );

        {
            auto inc = fold( trigger, 0, Incrementer<int>{} );
            for( auto i = 1; i <= 100; i++ )
                trigger.emit();

            CHECK_EQ( inc(), 100 );
        }

        {
            auto dec = fold( trigger, 100, Decrementer<int>{} );
            for( auto i = 1; i <= 100; i++ )
                trigger.emit();

            CHECK_EQ( dec(), 0 );
        }
    }

    TEST_CASE( "IterateByRef1" )
    {
        context ctx;

        auto src = make_event_source<int>( ctx );
        auto f = fold(
            src, std::vector<int>(), []( std::vector<int>& v, int d ) { v.push_back( d ); } );

        // Push
        for( auto i = 1; i <= 100; i++ )
            src << i;

        CHECK_EQ( f().size(), 100 );

        // Check
        for( auto i = 1; i <= 100; i++ )
            CHECK_EQ( f()[i - 1], i );
    }

    TEST_CASE( "IterateByRef2" )
    {
        context ctx;

        auto src = make_event_source( ctx );
        auto x = fold(
            src, std::vector<int>(), []( std::vector<int>& v, token ) { v.push_back( 123 ); } );

        // Push
        for( auto i = 0; i < 100; i++ )
            src.emit();

        CHECK_EQ( x().size(), 100 );

        // Check
        for( auto i = 0; i < 100; i++ )
            CHECK_EQ( x()[i], 123 );
    }

    TEST_CASE( "SyncedIterate1" )
    {
        context ctx;

        auto in1 = make_var( ctx, 1 );
        auto in2 = make_var( ctx, 1 );

        auto summ = []( int a, int b ) { return a + b; };

        auto op1 = ( in1, in2 ) | summ;
        auto op2 = ( ( in1, in2 ) | summ ) | []( int a ) { return a * 10; };

        auto src1 = make_event_source( ctx );
        auto src2 = make_event_source<int>( ctx );

        auto out1 = fold( src1,
            std::make_tuple( 0, 0 ),
            with( op1, op2 ),
            []( const std::tuple<int, int>& t, token, int op1, int op2 ) {
                return std::make_tuple<int, int>( std::get<0>( t ) + op1, std::get<1>( t ) + op2 );
            } );

        auto out2 = fold( src2,
            std::make_tuple( 0, 0, 0 ),
            with( op1, op2 ),
            []( const std::tuple<int, int, int>& t, int e, int op1, int op2 ) {
                return std::make_tuple<int, int, int>(
                    std::get<0>( t ) + e, std::get<1>( t ) + op1, std::get<2>( t ) + op2 );
            } );

        int obsCount1 = 0;
        int obsCount2 = 0;

        {
            auto obs1 = observe( out1, [&]( const std::tuple<int, int>& t ) {
                ++obsCount1;

                CHECK_EQ( std::get<0>( t ), 33 );
                CHECK_EQ( std::get<1>( t ), 330 );
            } );

            auto obs2 = observe( out2, [&]( const std::tuple<int, int, int>& t ) {
                ++obsCount2;

                CHECK_EQ( std::get<0>( t ), 42 );
                CHECK_EQ( std::get<1>( t ), 33 );
                CHECK_EQ( std::get<2>( t ), 330 );
            } );

            in1 <<= 22;
            in2 <<= 11;

            src1.emit();
            src2.emit( 42 );

            CHECK_EQ( obsCount1, 1 );
            CHECK_EQ( obsCount2, 1 );

            obs1.detach();
            obs2.detach();
        }

        {
            auto obs1 = observe( out1, [&]( const std::tuple<int, int>& t ) {
                ++obsCount1;

                CHECK_EQ( std::get<0>( t ), 33 + 330 );
                CHECK_EQ( std::get<1>( t ), 330 + 3300 );
            } );

            auto obs2 = observe( out2, [&]( const std::tuple<int, int, int>& t ) {
                ++obsCount2;

                CHECK_EQ( std::get<0>( t ), 42 + 420 );
                CHECK_EQ( std::get<1>( t ), 33 + 330 );
                CHECK_EQ( std::get<2>( t ), 330 + 3300 );
            } );

            in1 <<= 220;
            in2 <<= 110;

            src1.emit();
            src2.emit( 420 );

            CHECK_EQ( obsCount1, 2 );
            CHECK_EQ( obsCount2, 2 );

            obs1.detach();
            obs2.detach();
        }
    }

    TEST_CASE( "SyncedIterate2" )
    {
        context ctx;

        auto in1 = make_var( ctx, 1 );
        auto in2 = make_var( ctx, 1 );

        auto summ = []( int a, int b ) { return a + b; };

        auto op1 = ( in1, in2 ) | summ;
        auto op2 = ( ( in1, in2 ) | summ ) | []( int a ) { return a * 10; };

        auto src1 = make_event_source( ctx );
        auto src2 = make_event_source<int>( ctx );

        auto out1 = fold( src1,
            std::vector<int>{},
            with( op1, op2 ),
            []( std::vector<int>& v, token, int op1, int op2 ) -> void {
                v.push_back( op1 );
                v.push_back( op2 );
            } );

        auto out2 = fold( src2,
            std::vector<int>{},
            with( op1, op2 ),
            []( std::vector<int>& v, int e, int op1, int op2 ) -> void {
                v.push_back( e );
                v.push_back( op1 );
                v.push_back( op2 );
            } );

        int obsCount1 = 0;
        int obsCount2 = 0;

        {
            auto obs1 = observe( out1, [&]( const std::vector<int>& v ) {
                ++obsCount1;

                CHECK_EQ( v.size(), 2 );

                CHECK_EQ( v[0], 33 );
                CHECK_EQ( v[1], 330 );
            } );

            auto obs2 = observe( out2, [&]( const std::vector<int>& v ) {
                ++obsCount2;

                CHECK_EQ( v.size(), 3 );

                CHECK_EQ( v[0], 42 );
                CHECK_EQ( v[1], 33 );
                CHECK_EQ( v[2], 330 );
            } );

            in1 <<= 22;
            in2 <<= 11;

            src1.emit();
            src2.emit( 42 );

            CHECK_EQ( obsCount1, 1 );
            CHECK_EQ( obsCount2, 1 );

            obs1.detach();
            obs2.detach();
        }

        {
            auto obs1 = observe( out1, [&]( const std::vector<int>& v ) {
                ++obsCount1;

                CHECK_EQ( v.size(), 4 );

                CHECK_EQ( v[0], 33 );
                CHECK_EQ( v[1], 330 );
                CHECK_EQ( v[2], 330 );
                CHECK_EQ( v[3], 3300 );
            } );

            auto obs2 = observe( out2, [&]( const std::vector<int>& v ) {
                ++obsCount2;

                CHECK_EQ( v.size(), 6 );

                CHECK_EQ( v[0], 42 );
                CHECK_EQ( v[1], 33 );
                CHECK_EQ( v[2], 330 );

                CHECK_EQ( v[3], 420 );
                CHECK_EQ( v[4], 330 );
                CHECK_EQ( v[5], 3300 );
            } );

            in1 <<= 220;
            in2 <<= 110;

            src1.emit();
            src2.emit( 420 );

            CHECK_EQ( obsCount1, 2 );
            CHECK_EQ( obsCount2, 2 );

            obs1.detach();
            obs2.detach();
        }
    }

    TEST_CASE( "SyncedIterate3" )
    {
        context ctx;

        auto in1 = make_var( ctx, 1 );
        auto in2 = make_var( ctx, 1 );

        auto summ = []( int a, int b ) { return a + b; };

        auto op1 = ( in1, in2 ) | summ;
        auto op2 = ( ( in1, in2 ) | summ ) | []( int a ) { return a * 10; };

        auto src1 = make_event_source( ctx );
        auto src2 = make_event_source<int>( ctx );

        auto out1 = fold( src1,
            std::make_tuple( 0, 0 ),
            with( op1, op2 ),
            []( const std::tuple<int, int>& t, event_range<token> range, int op1, int op2 ) {
                return std::make_tuple<int, int>( //
                    std::get<0>( t ) + ( op1 * static_cast<int>( range.size() ) ),
                    std::get<1>( t ) + ( op2 * static_cast<int>( range.size() ) ) );
            } );

        auto out2 = fold( src2,
            std::make_tuple( 0, 0, 0 ),
            with( op1, op2 ),
            []( const std::tuple<int, int, int>& t, event_range<int> range, int op1, int op2 ) {
                int sum = 0;
                for( const auto& e : range )
                    sum += e;

                return std::make_tuple<int, int, int>( //
                    std::get<0>( t ) + sum,
                    std::get<1>( t ) + ( op1 * static_cast<int>( range.size() ) ),
                    std::get<2>( t ) + ( op2 * static_cast<int>( range.size() ) ) );
            } );

        int obsCount1 = 0;
        int obsCount2 = 0;

        {
            auto obs1 = observe( out1, [&]( const std::tuple<int, int>& t ) {
                ++obsCount1;

                CHECK_EQ( std::get<0>( t ), 33 );
                CHECK_EQ( std::get<1>( t ), 330 );
            } );

            auto obs2 = observe( out2, [&]( const std::tuple<int, int, int>& t ) {
                ++obsCount2;

                CHECK_EQ( std::get<0>( t ), 42 );
                CHECK_EQ( std::get<1>( t ), 33 );
                CHECK_EQ( std::get<2>( t ), 330 );
            } );

            in1 <<= 22;
            in2 <<= 11;

            src1.emit();
            src2.emit( 42 );

            CHECK_EQ( obsCount1, 1 );
            CHECK_EQ( obsCount2, 1 );

            obs1.detach();
            obs2.detach();
        }

        {
            auto obs1 = observe( out1, [&]( const std::tuple<int, int>& t ) {
                ++obsCount1;

                CHECK_EQ( std::get<0>( t ), 33 + 330 );
                CHECK_EQ( std::get<1>( t ), 330 + 3300 );
            } );

            auto obs2 = observe( out2, [&]( const std::tuple<int, int, int>& t ) {
                ++obsCount2;

                CHECK_EQ( std::get<0>( t ), 42 + 420 );
                CHECK_EQ( std::get<1>( t ), 33 + 330 );
                CHECK_EQ( std::get<2>( t ), 330 + 3300 );
            } );

            in1 <<= 220;
            in2 <<= 110;

            src1.emit();
            src2.emit( 420 );

            CHECK_EQ( obsCount1, 2 );
            CHECK_EQ( obsCount2, 2 );

            obs1.detach();
            obs2.detach();
        }
    }

    TEST_CASE( "SyncedIterate4" )
    {
        context ctx;

        auto in1 = make_var( ctx, 1 );
        auto in2 = make_var( ctx, 1 );

        auto summ = []( int a, int b ) { return a + b; };

        auto op1 = ( in1, in2 ) | summ;
        auto op2 = ( ( in1, in2 ) | summ ) | []( int a ) { return a * 10; };

        auto src1 = make_event_source( ctx );
        auto src2 = make_event_source<int>( ctx );

        auto out1 = fold( src1,
            std::vector<int>{},
            with( op1, op2 ),
            []( std::vector<int>& v, event_range<token> range, int op1, int op2 ) -> void {
                for( const auto& e : range )
                {
                    (void)e;
                    v.push_back( op1 );
                    v.push_back( op2 );
                }
            } );

        auto out2 = fold( src2,
            std::vector<int>{},
            with( op1, op2 ),
            []( std::vector<int>& v, event_range<int> range, int op1, int op2 ) -> void {
                for( const auto& e : range )
                {
                    v.push_back( e );
                    v.push_back( op1 );
                    v.push_back( op2 );
                }
            } );

        int obsCount1 = 0;
        int obsCount2 = 0;

        {
            auto obs1 = observe( out1, [&]( const std::vector<int>& v ) {
                ++obsCount1;

                CHECK_EQ( v.size(), 2 );

                CHECK_EQ( v[0], 33 );
                CHECK_EQ( v[1], 330 );
            } );

            auto obs2 = observe( out2, [&]( const std::vector<int>& v ) {
                ++obsCount2;

                CHECK_EQ( v.size(), 3 );

                CHECK_EQ( v[0], 42 );
                CHECK_EQ( v[1], 33 );
                CHECK_EQ( v[2], 330 );
            } );

            in1 <<= 22;
            in2 <<= 11;

            src1.emit();
            src2.emit( 42 );

            CHECK_EQ( obsCount1, 1 );
            CHECK_EQ( obsCount2, 1 );

            obs1.detach();
            obs2.detach();
        }

        {
            auto obs1 = observe( out1, [&]( const std::vector<int>& v ) {
                ++obsCount1;

                CHECK_EQ( v.size(), 4 );

                CHECK_EQ( v[0], 33 );
                CHECK_EQ( v[1], 330 );
                CHECK_EQ( v[2], 330 );
                CHECK_EQ( v[3], 3300 );
            } );

            auto obs2 = observe( out2, [&]( const std::vector<int>& v ) {
                ++obsCount2;

                CHECK_EQ( v.size(), 6 );

                CHECK_EQ( v[0], 42 );
                CHECK_EQ( v[1], 33 );
                CHECK_EQ( v[2], 330 );

                CHECK_EQ( v[3], 420 );
                CHECK_EQ( v[4], 330 );
                CHECK_EQ( v[5], 3300 );
            } );

            in1 <<= 220;
            in2 <<= 110;

            src1.emit();
            src2.emit( 420 );

            CHECK_EQ( obsCount1, 2 );
            CHECK_EQ( obsCount2, 2 );

            obs1.detach();
            obs2.detach();
        }
    }

    TEST_CASE( "SyncedEventProcess1" )
    {
        std::vector<float> results;

        context ctx;

        auto in1 = make_event_source<int>( ctx );
        auto in2 = make_event_source<int>( ctx );

        auto mult = make_var( ctx, 10 );

        auto merged = merge( in1, in2 );
        int callCount = 0;

        auto processed = process<float>( merged,
            with( mult ),
            [&]( event_range<int> range, event_emitter<float> out, int mult ) {
                for( const auto& e : range )
                {
                    *out = 0.1f * static_cast<float>( e * mult );
                    *out = 1.5f * static_cast<float>( e * mult );
                }

                callCount++;
            } );

        observe( processed, [&]( float s ) { results.push_back( s ); } );

        ctx.do_transaction( [&] { in1 << 10 << 20; } );

        in2 << 30;

        CHECK_EQ( results.size(), 6 );
        CHECK_EQ( callCount, 2 );

        CHECK_EQ( results[0], 10.0f );
        CHECK_EQ( results[1], 150.0f );
        CHECK_EQ( results[2], 20.0f );
        CHECK_EQ( results[3], 300.0f );
        CHECK_EQ( results[4], 30.0f );
        CHECK_EQ( results[5], 450.0f );
    }

    TEST_CASE( "Zip1" )
    {
        context ctx;

        auto in1 = make_event_source<int>( ctx );
        auto in2 = make_event_source<int>( ctx );

        auto zipped = zip( in1, in2 );

        std::vector<std::tuple<int, int>> saved_events;

        observe(
            zipped, [&]( const std::tuple<int, int>& value ) { saved_events.push_back( value ); } );

        in1 << -1 << -2 << -3;
        in2 << 1 << 2 << 3 << 4;

        const std::vector<std::tuple<int, int>> expected = {
            { -1, 1 },
            { -2, 2 },
            { -3, 3 },
        };

        CHECK( saved_events == expected );
    }

    TEST_CASE( "Zip2" )
    {
        context ctx;

        auto in1 = make_event_source<int>( ctx );
        auto in2 = make_event_source<int>( ctx );

        auto zipped = zip( in1, in2 );

        std::vector<std::tuple<int, int>> saved_events;

        observe(
            zipped, [&]( const std::tuple<int, int>& value ) { saved_events.push_back( value ); } );

        ctx.do_transaction( [&] {
            in1 << -1 << -2 << -3;
            in2 << 1 << 2 << 3 << 4;
        } );

        const std::vector<std::tuple<int, int>> expected = {
            { -1, 1 },
            { -2, 2 },
            { -3, 3 },
        };

        CHECK( saved_events == expected );
    }
}
