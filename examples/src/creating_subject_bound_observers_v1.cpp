#include <iostream>

#include "ureact/ureact.hpp"

int main()
{
    std::cout << "=====================================\n";
    std::cout << "Creating subject-bound observers (v1)\n";
    std::cout << "=====================================\n";
    std::cout << "\n";

    ureact::context c;

    auto x = make_var( c, 1 );

    {
        // Create a signal in the function scope
        auto mySignal = make_signal( x, []( int x_ ) { return x_; } );

        // The lifetime of the observer is bound to mySignal.
        // After Run() returns, mySignal is destroyed, and so is the observer
        observe( mySignal, []( int value ) { std::cout << value << "\n"; } );

        x <<= 2; // output: 2
    }

    x <<= 3; // no ouput
}
