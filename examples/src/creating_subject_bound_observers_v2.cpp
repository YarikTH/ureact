#include <iostream>

#include "ureact/ureact.hpp"

int main()
{
    std::cout << "=====================================\n";
    std::cout << "Creating subject-bound observers (v2)\n";
    std::cout << "=====================================\n";
    std::cout << "\n";

    ureact::context c;

    auto x = make_var( c, 1 );

    // Outer scope
    {
        // Unbound observer
        ureact::observer obs;

        // Inner scope
        {
            auto mySignal = make_signal( x, []( int x_ ) { return x_; } );

            // Move-assign to obs
            obs = observe( mySignal, []( int value ) { std::cout << value << "\n"; } );

            // The node linked to mySignal is now also owned by obs

            x <<= 2; // output: 2
        }
        // ~Inner scope

        // mySignal was destroyed, but as long as obs exists and is still
        // attached to the signal node, this signal node won't be destroyed

        x <<= 3; // output: 3
    }
    // ~Outer scope

    // obs was destroyed
    // -> the signal node is no longer owned by anything and is destroyed
    // -> the observer node is destroyed as it was bound to the subject

    x <<= 4; // no ouput
}
