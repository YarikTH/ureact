#include <doctest.h>
#include <string>
#include <vector>
#include <sstream>

#include "ureact/ureact.hpp"

namespace std
{
template <class T, class Allocator>
doctest::String toString( const std::vector<T, Allocator>& value )
{
    std::stringstream ss;

    ss << "[";
    for( auto it = value.begin(), ite = value.end(); it != ite; ++it )
    {
        ss << *it;
        if( it + 1 != ite )
        {
            ss << ", ";
        }
    }
    ss << "]";

    return ss.str().c_str();
}
} // namespace std

TEST_SUITE( "Examples" )
{
    TEST_CASE( "Hello World" )
    {
        ureact::context ctx;

        ureact::var_signal<std::string> firstWord = make_var( ctx, std::string( "Change" ) );
        ureact::var_signal<std::string> secondWord = make_var( ctx, std::string( "me!" ) );

        ureact::signal<std::string> bothWords;

        auto concatFunc = []( const std::string& first, const std::string& second ) -> std::string {
            return first + " " + second;
        };

        // Several alternative variants that do exactly the same
        SUBCASE( "using overloaded operators" )
        {
            bothWords = firstWord + " " + secondWord;
        }

        SUBCASE( "using make_signal()" )
        {
            bothWords = make_signal( with( firstWord, secondWord ), concatFunc );
        }

        SUBCASE( "operators , and ->*" )
        {
            // operator "," can be used instead of function with()
            // operator "->*" can be used instead of make_signal()
            bothWords = /*with*/ ( firstWord, secondWord )->*concatFunc;
        }

        // Imperative value access
        CHECK( bothWords.value() == "Change me!" );

        // Imperative value change
        firstWord <<= std::string( "Hello" );

        CHECK( bothWords.value() == "Hello me!" );

        secondWord <<= std::string( "World!" );

        CHECK( bothWords.value() == "Hello World!" );
    }

    TEST_CASE( "Modifying signal values in place" )
    {
        ureact::context ctx;

        ureact::var_signal<std::vector<std::string>> data
            = make_var( ctx, std::vector<std::string>{} );

        CHECK( data.value() == std::vector<std::string>{} );

        data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "Hello" ); } );

        CHECK( data.value() == std::vector<std::string>{ "Hello" } );

        data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "World!" ); } );

        CHECK( data.value() == std::vector<std::string>{ "Hello", "World!" } );
    }
}
