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

TEST_CASE( "LiftOperatorPriority" )
{
    // https://en.cppreference.com/w/cpp/language/operator_precedence
    // 3.  +a -a          Unary plus and minus
    //     ! ~            Logical NOT and bitwise NOT
    // 5.  a*b  a/b  a%b  Multiplication, division, and remainder
    // 6.  a+b  a-b       Addition and subtraction
    // 8.  <=>            Three-way comparison operator (since C++20)
    // 9.  <  <=          For relational operators < and ≤ respectively
    //     >  >=          For relational operators > and ≥ respectively
    // 10. ==  !=         For equality operators = and ≠ respectively
    // 14. &&             Logical AND
    // 15. ||             Logical OR

    ureact::context ctx;

    auto _2 = make_var( ctx, 2 );

    auto result = _2 + _2 * _2;
    CHECK( result.get() == 6 );

    auto result2 = ( _2 + _2 ) * _2;
    CHECK( result2.get() == 8 );
}
