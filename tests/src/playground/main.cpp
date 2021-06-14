#include "ureact/ureact.hpp"
#include <iostream>

int main()
{
    ureact::context ctx;

    ureact::var_signal<int> a = make_var( ctx, 1 );
    ureact::var_signal<int> b = make_var( ctx, 1 );

    ureact::signal<int> x = a + b;
    ureact::signal<int> y = a + b;
    ureact::signal<int> z = x + y; // 2*(a+b)

    //  a b  //
    //  /X\  //
    // x   y //
    //  \ /  //
    //   z   //

    a <<= 2; // z is changed to 6
    b <<= 2; // z is changed to 8

    ctx.do_transaction( [&] {
        a <<= 4;
        b <<= 4;
    } ); // z is changed to 16
}
