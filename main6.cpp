
#include <fmt/core.h>

#include "https://raw.githubusercontent.com/YarikTH/ureact/main/single_include/ureact/ureact_amalgamated.hpp"

int main()
{
    ureact::context ctx;

    auto foo = ureact::make_source<int>( ctx ); // error message when template parameter missed is weird
    auto bar = ureact::transform( foo, []( int value ) { return value + 1; } );

    // Subscribe to all notifiers to add verbosity
    observe( foo, []( int value ) { //
        fmt::print( "foo value is emitted {}\n", value );
    } );
    observe( bar, []( int value ) { //
        fmt::print( "bar value is emitted {}\n", value );
    } );

    auto separator = []() { fmt::print( "=================\n" ); };

    // change variables to see what happens
    foo( 1 );
    separator();

    foo( 2 );
    separator();
}
