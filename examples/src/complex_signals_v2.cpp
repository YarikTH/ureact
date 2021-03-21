#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "ureact/ureact.hpp"

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

void printExpressions( const ExprVectT& expressions )
{
    std::cout << "Expressions:\n";
    for ( const auto& p : expressions )
    {
        std::cout << "\t" << p.first << " is " << p.second << "\n";
    }
}

ureact::signal<ExprVectT> createExpressionSignal(
    const ureact::signal<int>& a, const ureact::signal<int>& b )
{
    // Inside a function, we can use auto
    const auto sumExpr = make_signal(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "+" ); } );

    const auto diffExpr = make_signal(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "-" ); } );

    const auto prodExpr = make_signal(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "*" ); } );

    return make_signal(
        // clang-format off
        with(

            make_signal( with( sumExpr,  a + b  ), &makeExprPair ),
            make_signal( with( diffExpr, a - b ), &makeExprPair ),
            make_signal( with( prodExpr, a * b ), &makeExprPair )
        ),
        // clang-format on
        []( const ExprPairT& sumP, const ExprPairT& diffP, const ExprPairT& prodP ) {
            return ExprVectT{ sumP, diffP, prodP };
        } );
}

int main()
{
    std::cout << "=======================================================\n";
    std::cout << "Complex signals (v2 intermediate signals in a function)\n";
    std::cout << "=======================================================\n";
    std::cout << "\n";

    ureact::context c;

    // Input operands
    ureact::var_signal<int> a = make_var( c, 1 );
    ureact::var_signal<int> b = make_var( c, 2 );

    // The expression std::vector
    ureact::signal<ExprVectT> expressions = createExpressionSignal( a, b );

    observe( expressions, printExpressions );

    a <<= 30;
    b <<= 40;

    std::cout << "\n";
}
