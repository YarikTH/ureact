#include "doctest/doctest.h"
#include "ureact/ureact.hpp"
#include <queue>
#include <string>

namespace
{

int myfunc( int a, int b )
{
    return a + b;
}
float myfunc2( int a )
{
    return static_cast<float>( a ) / 2.0f;
}
float myfunc3( float a, float b )
{
    return a * b;
}

} // namespace

TEST_SUITE_BEGIN( "SignalTest" );

TEST_CASE( "MakeVars" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 1 );
    auto v2 = make_var( ctx, 2 );
    auto v3 = make_var( ctx, 3 );
    auto v4 = make_var( ctx, 4 );

    CHECK( v1.value() == 1 );
    CHECK( v2.value() == 2 );
    CHECK( v3.value() == 3 );
    CHECK( v4.value() == 4 );

    v1 <<= 10;
    v2 <<= 20;
    v3 <<= 30;
    v4 <<= 40;

    CHECK( v1.value() == 10 );
    CHECK( v2.value() == 20 );
    CHECK( v3.value() == 30 );
    CHECK( v4.value() == 40 );
}

TEST_CASE( "Signals1" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 1 );
    auto v2 = make_var( ctx, 2 );
    auto v3 = make_var( ctx, 3 );
    auto v4 = make_var( ctx, 4 );

    auto s1 = make_signal( with( v1, v2 ), []( int a, int b ) { return a + b; } );

    auto s2 = make_signal( with( v3, v4 ), []( int a, int b ) { return a + b; } );

    auto s3 = s1 + s2;

    CHECK( s1.value() == 3 );
    CHECK( s2.value() == 7 );
    CHECK( s3.value() == 10 );

    v1 <<= 10;
    v2 <<= 20;
    v3 <<= 30;
    v4 <<= 40;

    CHECK( s1.value() == 30 );
    CHECK( s2.value() == 70 );
    CHECK( s3.value() == 100 );

    CHECK( ureact::is_signal<decltype( v1 )>::value );

    CHECK( ureact::is_signal<decltype( s1 )>::value );

    CHECK( ureact::is_signal<decltype( s2 )>::value );

    CHECK_FALSE( ureact::is_signal<decltype( 10 )>::value );
}

TEST_CASE( "Signals2" )
{
    ureact::context ctx;

    auto a1 = make_var( ctx, 1 );
    auto a2 = make_var( ctx, 1 );

    auto b1 = a1 + 0;
    auto b2 = a1 + a2;
    auto b3 = a2 + 0;

    auto c1 = b1 + b2;
    auto c2 = b2 + b3;

    auto result = c1 + c2;

    int observeCount = 0;

    observe( result, [&observeCount]( int v ) {
        observeCount++;
        if ( observeCount == 1 )
            CHECK( v == 9 );
        else
            CHECK( v == 12 );
    } );

    CHECK( a1.value() == 1 );
    CHECK( a2.value() == 1 );

    CHECK( b1.value() == 1 );
    CHECK( b2.value() == 2 );
    CHECK( b3.value() == 1 );

    CHECK( c1.value() == 3 );
    CHECK( c2.value() == 3 );

    CHECK( result.value() == 6 );

    a1 <<= 2;

    CHECK( observeCount == 1 );

    CHECK( a1.value() == 2 );
    CHECK( a2.value() == 1 );

    CHECK( b1.value() == 2 );
    CHECK( b2.value() == 3 );
    CHECK( b3.value() == 1 );

    CHECK( c1.value() == 5 );
    CHECK( c2.value() == 4 );

    CHECK( result.value() == 9 );

    a2 <<= 2;

    CHECK( observeCount == 2 );

    CHECK( a1.value() == 2 );
    CHECK( a2.value() == 2 );

    CHECK( b1.value() == 2 );
    CHECK( b2.value() == 4 );
    CHECK( b3.value() == 2 );

    CHECK( c1.value() == 6 );
    CHECK( c2.value() == 6 );

    CHECK( result.value() == 12 );
}

TEST_CASE( "Signals3" )
{
    ureact::context ctx;

    auto a1 = make_var( ctx, 1 );
    auto a2 = make_var( ctx, 1 );

    auto b1 = a1 + 0;
    auto b2 = a1 + a2;
    auto b3 = a2 + 0;

    auto c1 = b1 + b2;
    auto c2 = b2 + b3;

    auto result = c1 + c2;

    int observeCount = 0;

    observe( result, [&observeCount]( int v ) {
        observeCount++;
        CHECK( v == 12 );
    } );

    CHECK( a1.value() == 1 );
    CHECK( a2.value() == 1 );

    CHECK( b1.value() == 1 );
    CHECK( b2.value() == 2 );
    CHECK( b3.value() == 1 );

    CHECK( c1.value() == 3 );
    CHECK( c2.value() == 3 );

    CHECK( result.value() == 6 );

    ctx.do_transaction( [&] {
        a1 <<= 2;
        a2 <<= 2;
    } );

    CHECK( observeCount == 1 );

    CHECK( a1.value() == 2 );
    CHECK( a2.value() == 2 );

    CHECK( b1.value() == 2 );
    CHECK( b2.value() == 4 );
    CHECK( b3.value() == 2 );

    CHECK( c1.value() == 6 );
    CHECK( c2.value() == 6 );

    CHECK( result.value() == 12 );
}

TEST_CASE( "Signals4" )
{
    ureact::context ctx;

    auto a1 = make_var( ctx, 1 );
    auto a2 = make_var( ctx, 1 );

    auto b1 = a1 + a2;
    auto b2 = b1 + a2;

    CHECK( a1.value() == 1 );
    CHECK( a2.value() == 1 );

    CHECK( b1.value() == 2 );
    CHECK( b2.value() == 3 );

    a1 <<= 10;

    CHECK( a1.value() == 10 );
    CHECK( a2.value() == 1 );

    CHECK( b1.value() == 11 );
    CHECK( b2.value() == 12 );
}

TEST_CASE( "FunctionBind1" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 2 );
    auto v2 = make_var( ctx, 30 );
    auto v3 = make_var( ctx, 10 );

    auto signal = ( v1, v2, v3 )->*[=]( int a, int b, int c ) -> int { return a * b * c; };

    CHECK( signal.value() == 600 );
    v3 <<= 100;
    CHECK( signal.value() == 6000 );
}

TEST_CASE( "FunctionBind2" )
{
    ureact::context ctx;

    auto a = make_var( ctx, 1 );
    auto b = make_var( ctx, 1 );

    auto c = ( ( a + b ), ( a + 100 ) )->*&myfunc;
    auto d = c->*&myfunc2;
    auto e = ( d, d )->*&myfunc3;
    auto f = -e + 100.f;

    CHECK( c.value() == 103 );
    CHECK( d.value() == 51.5f );
    CHECK( e.value() == 2652.25f );
    CHECK( f.value() == -2552.25f );

    a <<= 10;

    CHECK( c.value() == 121 );
    CHECK( d.value() == 60.5f );
    CHECK( e.value() == 3660.25f );
    CHECK( f.value() == -3560.25f );
}

TEST_CASE( "Flatten1" )
{
    ureact::context ctx;

    auto inner1 = make_var( ctx, 123 );
    auto inner2 = make_var( ctx, 789 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = flatten( outer );

    std::queue<int> results;

    observe( flattened, [&]( int v ) { results.push( v ); } );

    CHECK( outer.value().equals( inner1 ) );
    CHECK( flattened.value() == 123 );

    inner1 <<= 456;

    CHECK( flattened.value() == 456 );

    CHECK( results.front() == 456 );
    results.pop();
    CHECK( results.empty() );

    outer <<= inner2;

    CHECK( outer.value().equals( inner2 ) );
    CHECK( flattened.value() == 789 );

    CHECK( results.front() == 789 );
    results.pop();
    CHECK( results.empty() );
}

TEST_CASE( "Flatten2" )
{
    ureact::context ctx;

    auto a0 = make_var( ctx, 100 );

    auto inner1 = make_var( ctx, 200 );

    auto a1 = make_var( ctx, 300 );
    auto a2 = a1 + 0;
    auto a3 = a2 + 0;
    auto a4 = a3 + 0;
    auto a5 = a4 + 0;
    auto a6 = a5 + 0;
    auto inner2 = a6 + 0;

    CHECK( inner1.value() == 200 );
    CHECK( inner2.value() == 300 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = flatten( outer );

    CHECK( flattened.value() == 200 );

    int observeCount = 0;

    observe( flattened, [&observeCount]( int /*unused*/ ) { observeCount++; } );

    auto o1 = a0 + flattened;
    auto o2 = o1 + 0;
    auto o3 = o2 + 0;
    auto result = o3 + 0;

    CHECK( result.value() == 100 + 200 );

    inner1 <<= 1234;

    CHECK( result.value() == 100 + 1234 );
    CHECK( observeCount == 1 );

    outer <<= inner2;

    CHECK( result.value() == 100 + 300 );
    CHECK( observeCount == 2 );

    ctx.do_transaction( [&] {
        a0 <<= 5000;
        a1 <<= 6000;
    } );

    CHECK( result.value() == 5000 + 6000 );
    CHECK( observeCount == 3 );
}

TEST_CASE( "Flatten3" )
{
    ureact::context ctx;

    auto inner1 = make_var( ctx, 10 );

    auto a1 = make_var( ctx, 20 );
    auto a2 = a1 + 0;
    auto a3 = a2 + 0;
    auto inner2 = a3 + 0;

    auto outer = make_var( ctx, inner1 );

    auto a0 = make_var( ctx, 30 );

    auto flattened = flatten( outer );

    int observeCount = 0;

    observe( flattened, [&observeCount]( int /*unused*/ ) { observeCount++; } );

    auto result = flattened + a0;

    CHECK( result.value() == 10 + 30 );
    CHECK( observeCount == 0 );

    ctx.do_transaction( [&] {
        inner1 <<= 1000;
        a0 <<= 200000;
        a1 <<= 50000;
        outer <<= inner2;
    } );

    CHECK( result.value() == 50000 + 200000 );
    CHECK( observeCount == 1 );

    ctx.do_transaction( [&] {
        a0 <<= 667;
        a1 <<= 776;
    } );

    CHECK( result.value() == 776 + 667 );
    CHECK( observeCount == 2 );

    ctx.do_transaction( [&] {
        inner1 <<= 999;
        a0 <<= 888;
    } );

    CHECK( result.value() == 776 + 888 );
    CHECK( observeCount == 2 );
}

TEST_CASE( "Flatten4" )
{
    ureact::context ctx;

    std::vector<int> results;

    auto a1 = make_var( ctx, 100 );
    auto inner1 = a1 + 0;

    auto a2 = make_var( ctx, 200 );
    auto inner2 = a2;

    auto a3 = make_var( ctx, 200 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = flatten( outer );

    auto result = flattened + a3;

    observe( result, [&]( int v ) { results.push_back( v ); } );

    ctx.do_transaction( [&] {
        a3 <<= 400;
        outer <<= inner2;
    } );

    CHECK( results.size() == 1 );

    CHECK( std::find( results.begin(), results.end(), 600 ) != results.end() );
}

TEST_CASE( "Flatten5" )
{
    ureact::context ctx;

    auto inner1 = make_var( ctx, 123 );
    auto inner2 = make_var( ctx, 123 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = flatten( outer );

    std::queue<int> results;

    observe( flattened, [&]( int v ) { results.push( v ); } );

    CHECK( outer.value().equals( inner1 ) );
    CHECK( flattened.value() == 123 );

    CHECK( results.empty() );

    outer <<= inner2;

    CHECK( outer.value().equals( inner2 ) );
    CHECK( flattened.value() == 123 );

    // flattened observer shouldn't trigger if value isn't changed
    CHECK( results.empty() );
}

TEST_CASE( "Member1" )
{
    ureact::context ctx;

    auto outer = make_var( ctx, 10 );
    auto inner = make_var( ctx, outer );

    auto flattened = inner.flatten();

    observe( flattened, []( int v ) { CHECK( v == 30 ); } );

    outer <<= 30;
}

TEST_CASE( "Modify1" )
{
    ureact::context ctx;

    auto v = make_var( ctx, std::vector<int>{} );

    int obsCount = 0;

    observe( v, [&]( const std::vector<int>& v_ ) {
        REQUIRE( v_.size() == 3 );
        CHECK( v_[0] == 30 );
        CHECK( v_[1] == 50 );
        CHECK( v_[2] == 70 );

        obsCount++;
    } );

    v.modify( []( std::vector<int>& v_ ) {
        v_.push_back( 30 );
        v_.push_back( 50 );
        v_.push_back( 70 );
    } );

    CHECK( obsCount == 1 );
}

TEST_CASE( "Modify2" )
{
    ureact::context ctx;

    auto v = make_var( ctx, std::vector<int>{} );

    int obsCount = 0;

    observe( v, [&]( const std::vector<int>& v_ ) {
        REQUIRE( v_.size() == 3 );
        CHECK( v_[0] == 30 );
        CHECK( v_[1] == 50 );
        CHECK( v_[2] == 70 );

        obsCount++;
    } );

    ctx.do_transaction( [&] {
        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 30 ); } );

        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 50 ); } );

        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 70 ); } );
    } );

    CHECK( obsCount == 1 );
}

TEST_CASE( "Modify3" )
{
    ureact::context ctx;

    auto vect = make_var( ctx, std::vector<int>{} );

    int obsCount = 0;

    observe( vect, [&]( const std::vector<int>& v ) {
        CHECK( v[0] == 30 );
        CHECK( v[1] == 50 );
        CHECK( v[2] == 70 );

        obsCount++;
    } );

    ctx.do_transaction( [&] {
        vect.set( std::vector<int>{ 30, 50 } );

        vect.modify( []( std::vector<int>& v ) { v.push_back( 70 ); } );
    } );

    CHECK( obsCount == 1 );
}

TEST_CASE( "Recursive transactions" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 1 );

    int observeCount = 0;

    observe( v1, [&observeCount]( int /*v*/ ) { observeCount++; } );

    ctx.do_transaction( [&] {
        v1 <<= 7;

        ctx.do_transaction( [&] { v1 <<= 4; } );

        v1 <<= 1;
        v1 <<= 2;
    } );

    CHECK( observeCount == 1 );
}

TEST_CASE( "UnaryOperators" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 1 );

    // clang-format off
    auto unary_plus         = +v1;
    auto unary_minus        = -v1;
    auto logical_negation   = !v1;
    auto bitwise_complement = ~v1;
    // clang-format on

    auto checkValues = [&]( std::initializer_list<int> valuesToTest ) {
        for ( const int& value : valuesToTest )
        {
            v1 <<= value;

            // clang-format off
            CHECK( unary_plus.value()         == (+value) );
            CHECK( unary_minus.value()        == (-value) );
            CHECK( logical_negation.value()   == (!value) );
            CHECK( bitwise_complement.value() == (~value) );
            // clang-format on
        }
    };

    checkValues( { 0, 1, -4, 654 } );
}

TEST_CASE( "BinaryOperators" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 0 );
    auto v2 = make_var( ctx, 1 );
    auto v3 = make_var( ctx, 0 );
    auto v4 = make_var( ctx, 1 );

    // clang-format off
    auto addition            = v1 + v2;
    auto subtraction         = v1 - v2;
    auto multiplication      = v1 * v2;
    auto bitwise_and         = v1 & v2;
    auto bitwise_or          = v1 | v2;
    auto bitwise_xor         = v1 ^ v2;
    auto bitwise_left_shift  = v1 << v2;
    auto bitwise_right_shift = v1 >> v2;
    auto equal               = v1 == v2;
    auto not_equal           = v1 != v2;
    auto less                = v1 < v2;
    auto less_equal          = v1 <= v2;
    auto greater             = v1 > v2;
    auto greater_equal       = v1 >= v2;
    auto logical_and         = v1 && v2;
    auto logical_or          = v1 || v2;

    auto division            = v3 / v4;
    auto modulo              = v3 % v4;
    // clang-format on

    auto checkValues = [&]( std::initializer_list<std::pair<int, int>> valuesToTest ) {
        for ( const auto& values : valuesToTest )
        {
            ctx.do_transaction( [&]() {
                v1 <<= values.first;
                v2 <<= values.second;
            } );

            // clang-format off
            CHECK( addition.value()            == (values.first +  values.second) );
            CHECK( subtraction.value()         == (values.first -  values.second) );
            CHECK( multiplication.value()      == (values.first *  values.second) );
            CHECK( equal.value()               == (values.first == values.second) );
            CHECK( not_equal.value()           == (values.first != values.second) );
            CHECK( less.value()                == (values.first <  values.second) );
            CHECK( less_equal.value()          == (values.first <= values.second) );
            CHECK( greater.value()             == (values.first >  values.second) );
            CHECK( greater_equal.value()       == (values.first >= values.second) );
            CHECK( logical_and.value()         == (values.first && values.second) );
            CHECK( logical_or.value()          == (values.first || values.second) );
            CHECK( bitwise_and.value()         == (values.first &  values.second) );
            CHECK( bitwise_or.value()          == (values.first |  values.second) );
            CHECK( bitwise_xor.value()         == (values.first ^  values.second) );
            CHECK( bitwise_left_shift.value()  == (values.first << values.second) );
            CHECK( bitwise_right_shift.value() == (values.first >> values.second) );
            // clang-format on
        }
    };

    auto checkDivisionValues = [&]( std::initializer_list<std::pair<int, int>> valuesToTest ) {
        for ( const auto& values : valuesToTest )
        {
            ctx.do_transaction( [&]() {
                v3 <<= values.first;
                v4 <<= values.second;
            } );
            // clang-format off
            CHECK( division.value()            == (values.first /  values.second) );
            CHECK( modulo.value()              == (values.first %  values.second) );
            // clang-format on
        }
    };

    checkValues( {
        { 2, 2 },
        { 3, -3 },
        { 0, 0 },
        { -4, 7 },
        { -8, -2 },
        { 0, -2 },
        { 5, 0 },
    } );

    checkDivisionValues( {
        { 2, 2 },
        { 3, -3 },
        { 8, 3 },
    } );
}

TEST_CASE( "OperatorsPriority" )
{
    // https://en.cppreference.com/w/cpp/language/operator_precedence
    // 3.  +a -a          Unary plus and minus
    //     ! ~            Logical NOT and bitwise NOT
    // 5.  a*b  a/b  a%b  Multiplication, division, and remainder
    // 6.  a+b  a-b       Addition and subtraction
    // 7.  <<  >>         Bitwise left shift and right shift
    // 8.  <=>            Three-way comparison operator (since C++20)
    // 9.  <  <=          For relational operators < and ≤ respectively
    //     >  >=          For relational operators > and ≥ respectively
    // 10. ==  !=         For equality operators = and ≠ respectively
    // 11. &              Bitwise AND
    // 12. ^              Bitwise XOR (exclusive or)
    // 13. |              Bitwise OR (inclusive or)
    // 14. &&             Logical AND
    // 15. ||             Logical OR

    ureact::context ctx;

    auto _2 = make_var( ctx, 2 );

    auto result = _2 + _2 * _2;
    CHECK( result.value() == 6 );

    auto result2 = ( _2 + _2 ) * _2;
    CHECK( result2.value() == 8 );
}

TEST_CASE( "ConstSignals" )
{
    ureact::context ctx;

    auto makeExprStr = []( const int lhs,
                           const std::string& op,
                           const int rhs,
                           const int resultSignal ) -> std::string {
        return std::to_string( lhs ) + op + std::to_string( rhs ) + "="
             + std::to_string( resultSignal );
    };

    auto a = make_var( ctx, 2 );
    auto b = make_var( ctx, 3 );

    auto sum_string = ( a, std::string( "+" ), b, ( a + b ) )->*makeExprStr;
    std::cout << "???\n";
    // @todo Understand what the hell happens with nodes lifetume
    // It seems that my node created under the hood dies just after creation
    // but nodes created from temp_signal continue to be alive for some reason

    CHECK( sum_string.value() == "2+3=5" );
}

TEST_SUITE_END();
