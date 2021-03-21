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
    std::cout << "Expressions: "
              << "\n";
    for ( const auto& p : expressions )
        std::cout << "\t" << p.first << " is " << p.second << "\n";
}

int main()
{
    std::cout << "=========================================\n";
    std::cout << "Complex signals (v1 intermediate signals)\n";
    std::cout << "=========================================\n";
    std::cout << "\n";

    ureact::context ctx;

    // Input operands
    ureact::var_signal<int> a = make_var( ctx, 1 );
    ureact::var_signal<int> b = make_var( ctx, 2 );

    // Calculations
    // clang-format off
    ureact::signal<int> sum  = a + b;
    ureact::signal<int> diff = a - b;
    ureact::signal<int> prod = a * b;
    // clang-format on

    // std::stringified expressions
    ureact::signal<std::string> sumExpr = make_signal(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "+" ); } );

    ureact::signal<std::string> diffExpr = make_signal(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "-" ); } );

    ureact::signal<std::string> prodExpr = make_signal(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "*" ); } );

    // The expression std::vector
    ureact::signal<ExprVectT> expressions = make_signal(
        // clang-format off
            with(
                make_signal( with( sumExpr,  sum  ), &makeExprPair ),
                make_signal( with( diffExpr, diff ), &makeExprPair ),
                make_signal( with( prodExpr, prod ), &makeExprPair )
            ),
            // clang-format on,
            []( const ExprPairT& sumP, const ExprPairT& diffP, const ExprPairT& prodP ) {
                return ExprVectT{ sumP, diffP, prodP };
            } );

    observe( expressions, printExpressions );

    a <<= 10;
    b <<= 20;

    std::cout << "\n";
}
