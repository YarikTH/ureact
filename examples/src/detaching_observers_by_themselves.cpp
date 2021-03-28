#include <iostream>

#include "ureact/ureact.hpp"

int main()
{
    std::cout << "=================================\n";
    std::cout << "Detaching observers by themselves\n";
    std::cout << "=================================\n";
    std::cout << "\n";

    ureact::context ctx;

    auto x = make_var( ctx, 1 );

    ureact::observer obs = observe( x, []( int value ) {
        if( value < 0 )
        {
            std::cout << value << " [detaching observer]\n";
            return ureact::observer_action::stop_and_detach;
        }
        else
        {
            std::cout << value << "\n";
            return ureact::observer_action::next;
        }
    } );

    x <<= 2; // output: 2

    x <<= -1; // output: -1 [detaching observer]

    x <<= 4; // no output
}
