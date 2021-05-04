#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

TEST_SUITE( "Examples" )
{
    TEST_CASE( "Subject-bound observers" )
    {
        ureact::context ctx;

        auto x = make_var( ctx, 1 );

        auto identity = []( const int value ) { return value; };

        std::vector<int> x_values;

        auto on_x_value_change = [&]( const int new_value ) { x_values.push_back( new_value ); };

        CHECK( x_values == std::vector<int>{} );

        SUBCASE( "v1" )
        {
            // my_signal scope
            {
                // Create a signal in the function scope
                auto my_signal = make_signal( x, identity );

                // The lifetime of the observer is bound to my_signal.
                // After scope my_signal is destroyed, and so is the observer
                observe( my_signal, on_x_value_change );

                x <<= 2; // output: 2

                CHECK( x_values == std::vector<int>{ 2 } );
            }
            // ~my_signal scope

            x <<= 3; // no ouput

            CHECK( x_values == std::vector<int>{ 2 } );
        }

        SUBCASE( "v2" )
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

                    x <<= 2; // output: 2

                    CHECK( x_values == std::vector<int>{ 2 } );
                }
                // ~Inner scope

                // my_signal was destroyed, but as long as obs exists and is still
                // attached to the signal node, this signal node won't be destroyed

                x <<= 3; // output: 3

                CHECK( x_values == std::vector<int>{ 2, 3 } );
            }
            // ~Outer scope

            // obs was destroyed
            // -> the signal node is no longer owned by anything and is destroyed
            // -> the observer node is destroyed as it was bound to the subject

            x <<= 4; // no ouput

            CHECK( x_values == std::vector<int>{ 2, 3 } );
        }
    }
}
