#include <boost/signals2.hpp>
#include <fmt/core.h>

class Foo : public boost::signals2::signal<void( int )>
{
public:
    Foo() = default;
};

class Bar : public boost::signals2::signal<void( int )>
{
public:
    Bar( Foo& foo )
    {
        foo.connect(              //
            [this]( int value ) { //
                operator()( value + 1 );
            } );
    }
};

/// Utility function to compatibility with ureact
template <class S, class F>
void observe( S&& s, F&& f )
{
    std::forward<S>( s ).connect( std::forward<F>( f ) );
}

int main()
{
    Foo foo;
    Bar bar{ foo };

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
