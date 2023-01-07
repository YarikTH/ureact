//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>

#include "boost/signals2.hpp"

// t = seconds + 1
// g = (t > seconds)

//      [seconds]           1  //
//     /         \          |  //
//    /      [t = seconds + 1] //
//   /         /               //
//  [g = (t > seconds)]        //
class GlitchesExample
{
public:
    GlitchesExample()
    {
        recalcT( true );
        recalcG( true );

        // Order matters. Only if recalcG is subscribed before recalcT glitch occures
        secondsChanged.connect( [this]( const auto& ) { recalcG( false ); } );
        tChanged.connect( [this]( const auto& ) { recalcG( false ); } );

        secondsChanged.connect( [this]( const auto& ) { recalcT( false ); } );
    }

    /// getter, setter and notifier for "seconds"
    [[nodiscard]] int seconds() const
    {
        return m_seconds;
    }

    void seconds( int value )
    {
        if( m_seconds != value )
        {
            m_seconds = value;
            secondsChanged( value );
        }
    }

    boost::signals2::signal<void( int )> secondsChanged;

    /// getter and notifier for "t"
    [[nodiscard]] int t() const
    {
        return m_t;
    }

    boost::signals2::signal<void( int )> tChanged;

    /// getter and notifier for "t"
    [[nodiscard]] bool g() const
    {
        return m_g;
    }

    boost::signals2::signal<void( bool )> gChanged;

private:
    void recalcT( bool initial )
    {
        const int newValue = m_seconds + 1;
        if( m_t != newValue )
        {
            m_t = newValue;
            if( !initial )
                tChanged( newValue );
        }
    }

    void recalcG( bool initial )
    {
        const bool newValue = m_t > m_seconds;
        if( m_g != newValue )
        {
            m_g = newValue;
            if( !initial )
                gChanged( newValue );
        }
    }

    int m_seconds = 0;
    int m_t = 0;
    bool m_g = false;
};

/// Utility function to compatibility with ureact
template <class S, class F>
void observe( S&& s, F&& f )
{
    std::forward<S>( s ).connect( std::forward<F>( f ) );
}

int main()
{
    GlitchesExample example;

    // Subscribe to all notifiers to add verbosity
    observe( example.secondsChanged, //
        []( int value ) { std::cout << "seconds is changed to " << value << "\n"; } );
    observe( example.tChanged, //
        []( int value ) { std::cout << "t is changed to " << value << "\n"; } );
    observe( example.gChanged, //
        [&]( bool value ) {
            std::cout << "> g is changed to " << value << ". ";
            std::cout << "[seconds: " << example.seconds() << ", t: " << example.t() << "]\n";
        } );

    auto separator = []() { std::cout << "=================\n"; };

    // change variables to see what happens
    example.seconds( 1 );
    separator();

    example.seconds( 2 );
    separator();
}
