#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "ureact/signal.hpp"
#include "ureact/observer.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Example 1 - Hello world
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace example1
{
    // The concat function
    std::string concatFunc(const std::string& first, const std::string& second) {
        return first + std::string(" ") + second;
    }

    // A signal that concatenates both words
    namespace v1
    {
        void Run()
        {
            std::cout << "Example 1 - Hello world (make_signal)" << std::endl;

            ureact::context c;
            
            // The two words
            ureact::var_signal<std::string>  firstWord   = make_var(&c, std::string("Change"));
            ureact::var_signal<std::string>  secondWord  = make_var(&c, std::string("me!"));
    
            ureact::signal<std::string>  bothWords = make_signal(with(firstWord,secondWord), concatFunc);
            
            // Imperative imperative value access
            std::cout  << bothWords.value() << std::endl;

            // Imperative imperative change
            firstWord  <<= std::string("Hello");

            std::cout  << bothWords.value() << std::endl;

            secondWord <<= std::string("World");

            std::cout  << bothWords.value() << std::endl;

            std::cout << std::endl;
        }
    }

    // Using overloaded operator + instead of explicit make_signal
    namespace v2
    {
        void Run()
        {
            std::cout << "Example 1 - Hello world (operators)" << std::endl;

            ureact::context c;
            
            // The two words
            ureact::var_signal<std::string>  firstWord   = make_var(&c, std::string("Change"));
            ureact::var_signal<std::string>  secondWord  = make_var(&c, std::string("me!"));
    
            ureact::signal<std::string> bothWords = firstWord + std::string(" ") + secondWord;
            
            std::cout  << bothWords.value() << std::endl;

            firstWord  <<= std::string("Hello");

            std::cout  << bothWords.value() << std::endl;

            secondWord <<= std::string("World");

            std::cout  << bothWords.value() << std::endl;

            std::cout << std::endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Example 2 - Reacting to value changes 
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace example2
{
    void Run()
    {
        std::cout << "Example 2 - Reacting to value changes" << std::endl;
        
        ureact::context c;
        
        ureact::var_signal<int> x    = make_var(&c, 1);
        ureact::signal<int>    xAbs = make_signal(x, [] (int value) { return abs(value); });
        
        observe(xAbs, [] (int new_value) {
            std::cout << "xAbs changed to " << new_value << std::endl;
        });

                    // initially x is 1
        x <<=  2;   // output: xAbs changed to 2
        x <<= -3;   // output: xAbs changed to 3
        x <<=  3;   // no output, xAbs is still 3

        std::cout << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Example 3 - Changing multiple inputs
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace example3
{
    void Run()
    {
        std::cout << "Example 3 - Changing multiple inputs" << std::endl;
        
        ureact::context c;
        
        ureact::var_signal<int> a = make_var(&c, 1);
        ureact::var_signal<int> b = make_var(&c, 1);
        
        ureact::signal<int>     x = a + b;
        ureact::signal<int>     y = a + b;
        ureact::signal<int>     z = x + y;
        
        observe(z, [] (int new_value) {
            std::cout << "z changed to " << new_value << std::endl;
        });

        a <<= 2; // output: z changed to 6
        b <<= 2; // output: z changed to 8

        c.do_transaction([&] {
            a <<= 4;
            b <<= 4; 
        }); // output: z changed to 16

        std::cout << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Example 4 - Modifying signal values in place
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace example4
{
    void Run()
    {
        std::cout << "Example 4 - Modifying signal values in place" << std::endl;

        ureact::context c;

        ureact::var_signal<std::vector<std::string>> data = make_var(&c, std::vector<std::string>{ });
        
        data.modify([] (std::vector<std::string>& value) {
            value.emplace_back("Hello");
        });

        data.modify([] (std::vector<std::string>& value) {
            value.emplace_back("World");
        });

        for (const auto& s : data.value())
            std::cout << s << " ";
        std::cout << std::endl;
        // output: Hell World

        std::cout << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Example 5 - Complex signals
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace example5
{
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
        std::cout << "Expressions: " << std::endl;
        for (const auto& p : expressions)
            std::cout << "\t" << p.first << " is " << p.second << std::endl;
    }

    // Version 1 - Intermediate signals
    namespace v1
    {
        void Run()
        {
            std::cout << "Example 5 - Complex signals (v1)" << std::endl;

            ureact::context c;
            
            // Input operands
            ureact::var_signal<int> a = make_var(&c, 1);
            ureact::var_signal<int> b = make_var(&c, 2);
    
            // Calculations
            ureact::signal<int> sum  = a + b;
            ureact::signal<int> diff = a - b;
            ureact::signal<int> prod = a * b;
            
            // std::stringified expressions
            ureact::signal<std::string> sumExpr =
                make_signal(with(a,b),
                            []( const int lhs, const int rhs )
                            {
                                return makeExprStr(lhs, rhs, "+");
                            });
    
            ureact::signal<std::string> diffExpr =
                make_signal(with(a,b),
                            []( const int lhs, const int rhs )
                            {
                                return makeExprStr(lhs, rhs, "-");
                            });
    
            ureact::signal<std::string> prodExpr =
                make_signal(with(a,b),
                            []( const int lhs, const int rhs )
                            {
                                return makeExprStr(lhs, rhs, "*");
                            });
    
            // The expression std::vector
            ureact::signal<ExprVectT> expressions = make_signal(
                with(
                    make_signal(with(sumExpr, sum),   &makeExprPair),
                    make_signal(with(diffExpr, diff), &makeExprPair),
                    make_signal(with(prodExpr, prod), &makeExprPair)
                ),
                [] (const ExprPairT& sumP, const ExprPairT& diffP, const ExprPairT& prodP) {
                    return ExprVectT{ sumP, diffP, prodP};
                });
            
            observe(expressions, printExpressions);

            a <<= 10;
            b <<= 20;

            std::cout << std::endl;
        }
    }

    // Version 2 - Intermediate signals in a function
    namespace v2
    {
        ureact::signal<ExprVectT> createExpressionSignal(const ureact::signal<int>& a, const ureact::signal<int>& b)
        {
            // Inside a function, we can use auto
            const auto sumExpr =
                make_signal(with(a,b),
                            []( const int lhs, const int rhs )
                            {
                                return makeExprStr(lhs, rhs, "+");
                            });
    
            const auto diffExpr =
                make_signal(with(a,b),
                            []( const int lhs, const int rhs )
                            {
                                return makeExprStr(lhs, rhs, "-");
                            });
    
            const auto prodExpr =
                make_signal(with(a,b),
                            []( const int lhs, const int rhs )
                            {
                                return makeExprStr(lhs, rhs, "*");
                            });

            return make_signal(
                with(
                    make_signal(with(sumExpr,  a + b), &makeExprPair),
                    make_signal(with(diffExpr, a - b), &makeExprPair),
                    make_signal(with(prodExpr, a * b), &makeExprPair)
                ),
                [] (const ExprPairT& sumP, const ExprPairT& diffP, const ExprPairT& prodP) {
                    return ExprVectT{ sumP, diffP, prodP };
                });
        }
        
        void Run()
        {
            std::cout << "Example 5 - Complex signals (v2)" << std::endl;

            ureact::context c;
            
            // Input operands
            ureact::var_signal<int> a = make_var(&c, 1);
            ureact::var_signal<int> b = make_var(&c, 2);
    
            // The expression std::vector
            ureact::signal<ExprVectT> expressions = createExpressionSignal(a, b);
            
            observe(expressions, printExpressions);

            a <<= 30;
            b <<= 40;

            std::cout << std::endl;
        }
    }

    // Version 3 - Imperative function
    namespace v3
    {
        void Run()
        {
            std::cout << "Example 5 - Complex signals (v3)" << std::endl;

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

            std::cout << std::endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Run examples
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    example1::v1::Run();
    example1::v2::Run();

    example2::Run();

    example3::Run();

    example4::Run();

    example5::v1::Run();
    example5::v2::Run();
    example5::v3::Run();

    return 0;
}