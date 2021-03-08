#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "ureact/ureact.hpp"

// Helpers
using ExprPairT = std::pair<std::string,int>;
using ExprVectT = std::vector<ExprPairT>;

std::string makeExprStr(int a, int b, const char* op)
{
    return std::to_string(a) + std::string(op) + std::to_string(b);
}

ExprPairT makeExprPair(const std::string& s, int v)
{
    return make_pair(s, v);
}

void printExpressions(const ExprVectT& expressions)
{
    std::cout << "Expressions: " << "\n";
    for (const auto& p : expressions)
        std::cout << "\t" << p.first << " is " << p.second << "\n";
}

int main()
{
    std::cout << "========================================\n";
    std::cout << "Complex signals (v3 imperative function)\n";
    std::cout << "========================================\n";
    std::cout << "\n";
    
    ureact::context c;
    
    // Input operands
    ureact::var_signal<int> a = make_var(&c, 1);
    ureact::var_signal<int> b = make_var(&c, 2);

    // The expression std::vector
    ureact::signal<ExprVectT> expressions = make_signal(with(a,b), [] (int a_, int b_) {
        ExprVectT result;

        result.push_back(
            make_pair(
                makeExprStr(a_, b_, "+"),
                a_ + b_));

        result.push_back(
            make_pair(
                makeExprStr(a_, b_, "-"),
                a_ - b_));

        result.push_back(
            make_pair(
                makeExprStr(a_, b_, "*"),
                a_ * b_));

        return result;
    });
    
    observe(expressions, printExpressions);

    a <<= 50;
    b <<= 60;

    std::cout << "\n";
}
