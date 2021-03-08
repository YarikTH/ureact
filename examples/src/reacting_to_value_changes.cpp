#include <cmath>
#include <iostream>

#include "ureact/ureact.hpp"

int main()
{
    std::cout << "=========================\n";
    std::cout << "Reacting to value changes\n";
    std::cout << "=========================\n";
    std::cout << "\n";
    
    ureact::context c;
    
    ureact::var_signal<int> x    = make_var(&c, 1);
    ureact::signal<int> xAbs = make_signal(x, [] (int value) { return abs(value); });
    
    observe(xAbs, [] (int new_value) {
        std::cout << "xAbs changed to " << new_value << "\n";
    });

                // initially x is 1
    x <<=  2;   // output: xAbs changed to 2
    x <<= -3;   // output: xAbs changed to 3
    x <<=  3;   // no output, xAbs is still 3
}
