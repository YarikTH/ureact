#include <iostream>
#include <string>
#include <vector>

#include "ureact/ureact.hpp"

int main()
{
    std::cout << "================================\n";
    std::cout << "Modifying signal values in place\n";
    std::cout << "================================\n";
    std::cout << "\n";

    ureact::context ctx;

    ureact::var_signal<std::vector<std::string>> data = make_var( ctx, std::vector<std::string>{} );

    data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "Hello" ); } );

    data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "World" ); } );

    for ( const auto& s : data.value() )
        std::cout << s << " ";
    std::cout << "\n";
    // output: Hello World
}
