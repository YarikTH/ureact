#include <iostream>
#include <string>

#include "ureact/ureact.hpp"

int main()
{
    std::cout << "=======================\n";
    std::cout << "Hello world (operators)\n";
    std::cout << "=======================\n";
    std::cout << "\n";

    ureact::context ctx;

    // The two words
    ureact::var_signal<std::string> firstWord = make_var( ctx, std::string( "Change" ) );
    ureact::var_signal<std::string> secondWord = make_var( ctx, std::string( "me!" ) );

    ureact::signal<std::string> bothWords = firstWord + std::string( " " ) + secondWord;

    std::cout << bothWords.value() << "\n";

    firstWord <<= std::string( "Hello" );

    std::cout << bothWords.value() << "\n";

    secondWord <<= std::string( "World" );

    std::cout << bothWords.value() << "\n";

    std::cout << "\n";
}
