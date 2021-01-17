#include <iostream>

#include "ureact/ureact.hpp"

int main()
{
    std::cout << "============================\n";
    std::cout << "Detaching observers manually\n";
    std::cout << "============================\n";
    std::cout << "\n";

    ureact::context c;

    auto x = make_var(&c, 1);
    
    ureact::observer obs = observe(x, [] (int /*unused*/) {
        std::cout << "Triggered!\n";
    });

    x <<= 2; // output: Triggered!

    obs.detach();   // remove the observer

    x <<= 3; // no output
}
