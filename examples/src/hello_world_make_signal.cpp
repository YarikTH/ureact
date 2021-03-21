#include <iostream>
#include <string>

#include "ureact/ureact.hpp"

// The concat function
std::string concatFunc( const std::string& first, const std::string& second )
{
    return first + std::string( " " ) + second;
}

int main()
{
    std::cout << "=========================\n";
    std::cout << "Hello world (make_signal)\n";
    std::cout << "=========================\n";
    std::cout << "\n";

    ureact::context c;

    // The two words
    // clang-format off
    ureact::var_signal<std::string> firstWord  = make_var( c, std::string( "Change" ) );
    ureact::var_signal<std::string> secondWord = make_var( c, std::string( "me!" ) );
    // clang-format on

    ureact::signal<std::string> bothWords
        = make_signal( with( firstWord, secondWord ), concatFunc );

    // clang-format off
    // Imperative value access
    std::cout  << bothWords.value() << "\n";

    // Imperative value change
    firstWord  <<= std::string( "Hello" );
    std::cout  << bothWords.value() << "\n";

    secondWord <<= std::string( "World" );
    std::cout  << bothWords.value() << "\n";
    // clang-format on

    std::cout << "\n";
}
