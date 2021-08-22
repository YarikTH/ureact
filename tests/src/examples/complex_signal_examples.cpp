//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

namespace
{
// Helpers
using ExprPairT = std::pair<std::string, int>;
using ExprVectT = std::vector<ExprPairT>;

std::string makeExprStr( int a, int b, const char* op )
{
    return std::to_string( a ) + std::string( op ) + std::to_string( b );
}

ExprPairT makeExprPair( const std::string& s, int v )
{
    return make_pair( s, v );
}

std::string makeExpressionsString( const ExprVectT& expressions )
{
    std::ostringstream ss;
    ss << "Expressions:\n";
    for( const auto& p : expressions )
    {
        ss << "* " << p.first << " is " << p.second << "\n";
    }
    return ss.str();
}

ureact::function<ExprVectT> createExpressionSignal(
    const ureact::function<int>& a, const ureact::function<int>& b )
{
    // Inside a function, we can use auto
    const auto sumExpr = make_function(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "+" ); } );

    const auto diffExpr = make_function(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "-" ); } );

    const auto prodExpr = make_function(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "*" ); } );

    return make_function(
        // clang-format off
            with(
                make_function( with( sumExpr,  a + b  ), &makeExprPair ),
                make_function( with( diffExpr, a - b ),  &makeExprPair ),
                make_function( with( prodExpr, a * b ),  &makeExprPair )
            ),
        // clang-format on
        []( const ExprPairT& sumP, const ExprPairT& diffP, const ExprPairT& prodP ) {
            return ExprVectT{ sumP, diffP, prodP };
        } );
}
} // namespace

TEST_SUITE( "Examples" )
{
    TEST_CASE( "Complex signals" )
    {
        ureact::context ctx;

        // Input operands
        ureact::value<int> a = make_value( ctx, 1 );
        ureact::value<int> b = make_value( ctx, 2 );

        // The expression std::vector
        ureact::function<ExprVectT> expressions;

        // Several alternative variants that do exactly the same
        SUBCASE( "intermediate signals" )
        {
            // Calculations
            ureact::function<int> sum = a + b;
            ureact::function<int> diff = a - b;
            ureact::function<int> prod = a * b;

            // stringified expressions
            ureact::function<std::string> sumExpr = make_function( with( a, b ),
                []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "+" ); } );

            ureact::function<std::string> diffExpr = make_function( with( a, b ),
                []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "-" ); } );

            ureact::function<std::string> prodExpr = make_function( with( a, b ),
                []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "*" ); } );

            expressions = make_function(
                // clang-format off
                with(
                    make_function( with( sumExpr,  sum  ), &makeExprPair ),
                    make_function( with( diffExpr, diff ), &makeExprPair ),
                    make_function( with( prodExpr, prod ), &makeExprPair )
                ),
                // clang-format on
                []( const ExprPairT& sumP, const ExprPairT& diffP, const ExprPairT& prodP ) {
                    return ExprVectT{ sumP, diffP, prodP };
                } );
        }

        SUBCASE( "intermediate signals in a function" )
        {
            expressions = createExpressionSignal( a, b );
        }

        SUBCASE( "imperative function" )
        {
            expressions = make_function( with( a, b ), []( int a_, int b_ ) {
                return ExprVectT{
                    make_pair( makeExprStr( a_, b_, "+" ), a_ + b_ ),
                    make_pair( makeExprStr( a_, b_, "-" ), a_ - b_ ),
                    make_pair( makeExprStr( a_, b_, "*" ), a_ * b_ ),
                };
            } );
        }

        REQUIRE( expressions.is_valid() );

        ureact::function<std::string> expressionsString
            = make_function( expressions, &makeExpressionsString );

        CHECK( expressionsString.get() ==
               R"(Expressions:
* 1+2 is 3
* 1-2 is -1
* 1*2 is 2
)" );

        a <<= 10;

        CHECK( expressionsString.get() ==
               R"(Expressions:
* 10+2 is 12
* 10-2 is 8
* 10*2 is 20
)" );

        b <<= 20;

        CHECK( expressionsString.get() ==
               R"(Expressions:
* 10+20 is 30
* 10-20 is -10
* 10*20 is 200
)" );
    }
}
