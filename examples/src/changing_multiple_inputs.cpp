#include <iostream>

#include "ureact/ureact.hpp"

int main()
{
    std::cout << "========================\n";
    std::cout << "Changing multiple inputs\n";
    std::cout << "========================\n";
    std::cout << "\n";

    ureact::context c;

    ureact::var_signal<int> a = make_var( &c, 1 );
    ureact::var_signal<int> b = make_var( &c, 1 );

    ureact::signal<int> x = a + b;
    ureact::signal<int> y = a + b;
    ureact::signal<int> z = x + y;

    observe( z, []( int new_value ) { std::cout << "z changed to " << new_value << "\n"; } );

    a <<= 2; // output: z changed to 6
    b <<= 2; // output: z changed to 8

    c.do_transaction( [&] {
        a <<= 4;
        b <<= 4;
    } ); // output: z changed to 16
}
