#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

TEST_SUITE( "Examples" )
{
    TEST_CASE( "Observers" )
    {
        ureact::context ctx;

        auto x = make_var( ctx, 0 );

        auto identity = []( const int value ) { return value; };

        std::vector<int> x_values;

        auto on_x_value_change = [&]( const int new_value ) { x_values.push_back( new_value ); };

        CHECK( x_values == std::vector<int>{} );

        SUBCASE( "Subject-bound observers v1" )
        {
            // Inner scope
            {
                // Create a signal in the function scope
                auto my_signal = make_signal( x, identity );

                // The lifetime of the observer is bound to my_signal.
                // After scope my_signal is destroyed, and so is the observer
                observe( my_signal, on_x_value_change );

                x <<= 1; // output: 2

                CHECK( x_values == std::vector<int>{ 1 } );
            }
            // ~Inner scope

            x <<= 2; // no output

            CHECK( x_values == std::vector<int>{ 1 } );
        }

        SUBCASE( "Subject-bound observers v2" )
        {
            // Outer scope
            {
                // Unbound observer
                ureact::observer obs;

                // Inner scope
                {
                    auto my_signal = make_signal( x, identity );

                    // Move-assign to obs
                    obs = observe( my_signal, on_x_value_change );

                    // The node linked to my_signal is now also owned by obs

                    x <<= 1; // output: 2

                    CHECK( x_values == std::vector<int>{ 1 } );
                }
                // ~Inner scope

                // my_signal was destroyed, but as long as obs exists and is still
                // attached to the signal node, this signal node won't be destroyed

                x <<= 2; // output: 3

                CHECK( x_values == std::vector<int>{ 1, 2 } );
            }
            // ~Outer scope

            // obs was destroyed
            // -> the signal node is no longer owned by anything and is destroyed
            // -> the observer node is destroyed as it was bound to the subject

            x <<= 3; // no output

            CHECK( x_values == std::vector<int>{ 1, 2 } );
        }

        SUBCASE( "Detaching observers manually" )
        {
            ureact::observer obs = observe( x, on_x_value_change );

            x <<= 1; // output: 2

            CHECK( x_values == std::vector<int>{ 1 } );

            obs.detach(); // detach the observer

            x <<= 2; // no output

            CHECK( x_values == std::vector<int>{ 1 } );
        }

        SUBCASE( "Using scoped observer" )
        {
            // Note the semantic difference between scoped_observer and observer.
            //
            // During its lifetime, the observer handle of an observer guarantees that the
            // observed subject will not be destroyed and allows explicit detach.
            // But even after the observer handle is destroyed, the subject may continue to exist
            // and so will the observer.
            //
            // scoped_observer has similar semantics to a scoped lock.
            // When it's destroyed, it detaches and destroys the observer.

            // Inner scope
            {
                ureact::scoped_observer scoped_obs = observe( x, on_x_value_change );

                x <<= 1; // output: 1

                CHECK( x_values == std::vector<int>{ 1 } );
            }
            // ~Inner scope

            x <<= 2; // no output

            CHECK( x_values == std::vector<int>{ 1 } );
        }

        SUBCASE( "Detaching observers using return value" )
        {
            // Functor used for observer can optionally return value
            // Using this value observer can be optionally self detached
            ureact::observer obs = observe( x, [&]( const int v ) {
                if( v < 0 )
                {
                    return ureact::observer_action::stop_and_detach;
                }
                else
                {
                    x_values.push_back( v );
                    return ureact::observer_action::next;
                }
            } );

            x <<= 1;
            x <<= 2;
            x <<= 3;
            x <<= -1;
            x <<= 4;

            CHECK( x_values == std::vector<int>{ 1, 2, 3 } );
        }
    }
}
