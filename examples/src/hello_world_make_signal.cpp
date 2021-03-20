#include <iostream>
#include <string>

#include "ureact/ureact.hpp"

// The concat function
std::string concatFunc(const std::string& first, const std::string& second) {
    return first + std::string(" ") + second;
}

int main()
{
    std::cout << "=========================\n";
    std::cout << "Hello world (make_signal)\n";
    std::cout << "=========================\n";
    std::cout << "\n";
    
    ureact::context c;
    
    // The two words
    ureact::var_signal<std::string>  firstWord   = make_var(&c, std::string("Change"));
    ureact::var_signal<std::string>  secondWord  = make_var(&c, std::string("me!"));

    ureact::signal<std::string>  bothWords = make_signal(with(firstWord,secondWord), concatFunc);
    
    // Imperative imperative value access
    std::cout  << bothWords.value() << "\n";

    // Imperative imperative change
    firstWord  <<= std::string("Hello");

    std::cout  << bothWords.value() << "\n";

    secondWord <<= std::string("World");

    std::cout  << bothWords.value() << "\n";

    std::cout << "\n";
}