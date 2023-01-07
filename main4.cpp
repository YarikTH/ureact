#include <fmt/core.h>

#include "https://raw.githubusercontent.com/YarikTH/ureact/main/single_include/ureact/ureact_amalgamated.hpp"

int main()
{
    ureact::context ctx;

    auto foo = ureact::make_var( ctx, 0 );
    auto bar = foo + 1;

    // Subscribe to all notifiers to add verbosity
    foo.valueChanged.connect(
        []( int value ) { fmt::print( "foo value is changed to {}\n", value ); } );
    bar.valueChanged.connect(
        []( int value ) { fmt::print( "bar value is changed to {}\n", value ); } );

    auto separator = []() { fmt::print( "=================\n" ); };

    // change variables to see what happens
    foo.setValue( 1 );
    separator();

    foo.setValue( 2 );
    separator();
}
