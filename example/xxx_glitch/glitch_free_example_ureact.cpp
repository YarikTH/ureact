//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>

#include "ureact/lift.hpp"
#include "ureact/observe.hpp"
#include "ureact/ureact.hpp"

// t = seconds + 1
// g = (t > seconds)

//      [seconds]           1  //
//     /         \          |  //
//    /      [t = seconds + 1] //
//   /         /               //
//  [g = (t > seconds)]        //
class GlitchesExample
{
public:
    GlitchesExample( ureact::context& ctx )
    {
        seconds = ureact::make_var( ctx, 0 );
        t = seconds + 1;
        g = t > seconds;
    }

    ureact::var_signal<int> seconds;
    ureact::signal<int> t;
    ureact::signal<bool> g;
};

int main()
{
    ureact::context ctx;

    GlitchesExample example{ ctx };

    // Subscribe to all notifiers to add verbosity
    observe( example.seconds, //
        []( int value ) { std::cout << "seconds is changed to " << value << "\n"; } );
    observe( example.t, //
        []( int value ) { std::cout << "t is changed to " << value << "\n"; } );
    observe( example.g, //
        [&]( bool value ) {
            std::cout << "> g is changed to " << value << ". ";
            std::cout << "[seconds: " << example.seconds() << ", t: " << example.t() << "]\n";
        } );

    auto separator = []() { std::cout << "=================\n"; };

    // change variables to see what happens
    example.seconds( 1 );
    separator();

    example.seconds( 2 );
    separator();
}
