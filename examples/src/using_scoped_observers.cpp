#include <iostream>

#include "ureact/ureact.hpp"

int main()
{
    std::cout << "=====================\n";
    std::cout << "Using scoped observer\n";
    std::cout << "=====================\n";
    std::cout << "\n";

    ureact::context c;

    auto x = make_var( &c, 1 );

    // Inner scope
    {
        ureact::scoped_observer scopedObs
            = observe( x, []( int /*unused*/ ) { std::cout << "Triggered!\n"; } );

        x <<= 2; // output: Triggered!
    }
    // ~Inner scope

    x <<= 3; // no output

    // Note the semantic difference between scoped_observer and observer.
    //
    // During its lifetime, the observer handle of an observer guarantees that the
    // observed subject will not be destroyed and allows explicit detach.
    // But even after the observer handle is destroyed, the subject may continue to exist
    // and so will the observer.
    //
    // scoped_observer has similar semantics to a scoped lock.
    // When it's destroyed, it detaches and destroys the observer.
}
