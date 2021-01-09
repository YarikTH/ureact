#include <iostream>
#include <string>
#include <utility>

#include "ureact/signal.hpp"
#include "ureact/observer.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Example 1 - Creating subject-bound observers
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace example1
{
    namespace v1
    {
        void Run()
        {
            std::cout << "Example 1 - Creating subject-bound observers (v1)" << std::endl;

            ureact::context c;
    
            auto x = make_var(&c, 1);
            
            {
                // Create a signal in the function scope
                auto mySignal = make_signal(x, [] (int x_) { return x_; } );

                // The lifetime of the observer is bound to mySignal.
                // After Run() returns, mySignal is destroyed, and so is the observer
                observe(mySignal, [] (int value) {
                    std::cout << value << std::endl;
                });

                x <<= 2; // output: 2
            }

            x <<= 3; // no ouput

            std::cout << std::endl;
        }
    }

    namespace v2
    {
        void Run()
        {
            std::cout << "Example 1 - Creating subject-bound observers (v2)" << std::endl;

            ureact::context c;
    
            auto x = make_var(&c, 1);
            
            // Outer scope
            {
                // Unbound observer
                ureact::observer obs;

                // Inner scope
                {
                    auto mySignal = make_signal(x, [] (int x_) { return x_; } );

                    // Move-assign to obs
                    obs = observe(mySignal, [] (int value) {
                        std::cout << value << std::endl;
                    });

                    // The node linked to mySignal is now also owned by obs

                    x <<= 2; // output: 2
                } 
                // ~Inner scope

                // mySignal was destroyed, but as long as obs exists and is still
                // attached to the signal node, this signal node won't be destroyed

                x <<= 3; // output: 3
            }
            // ~Outer scope

            // obs was destroyed
            // -> the signal node is no longer owned by anything and is destroyed
            // -> the observer node is destroyed as it was bound to the subject

            x <<= 4; // no ouput

            std::cout << std::endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Example 2 - Detaching observers manually
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace example2
{
    void Run()
    {
        std::cout << "Example 2 - Detaching observers manually" << std::endl;

        ureact::context c;

        auto x = make_var(&c, 1);
        
        ureact::observer obs = observe(x, [] (int /*unused*/) {
            std::cout << "Triggered!" << std::endl;
        });

        x <<= 13; // output: Triggered!

        obs.detach();   // remove the observer

        x <<= 14; // no output

        std::cout << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Example 3 - Using scoped observers
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace example3
{
    void Run()
    {
        std::cout << "Example 3 - Using scoped observers" << std::endl;

        ureact::context c;
    
        auto x = make_var(&c, 1);
        
        // Inner scope
        {
            ureact::scoped_observer scopedObs
            (
                observe(x, [] (int /*unused*/) {
                    std::cout << "Triggered!" << std::endl;
                })
            );

            x <<= 13; // output: Triggered!
        }
        // ~Inner scope

        x <<= 14; // no output

        // Note the semantic difference between scoped_observer and observer.
        //
        // During its lifetime, the observer handle of an observer guarantees that the
        // observed subject will not be destroyed and allows explicit detach.
        // But even after the observer handle is destroyed, the subject may continue to exist
        // and so will the observer.
        //
        // scoped_observer has similar semantics to a scoped lock.
        // When it's destroyed, it detaches and destroys the observer.

        std::cout << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Run examples
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    example1::v1::Run();
    example1::v2::Run();

    example2::Run();

    example3::Run();

    return 0;
}