#include <boost/signals2.hpp>
#include <fmt/core.h>

// Class wraps integer value, provides
class Foo
{
public:
    Foo( int value )
        : m_value( value )
    {}

    [[nodiscard]] int value() const
    {
        return m_value;
    }

    void setValue( int value )
    {
        if( m_value != value )
        {
            m_value = value;
            valueChanged( value );
        }
    }

    boost::signals2::signal<void( int )> valueChanged;

private:
    int m_value;
};

class Bar
{
public:
    Bar( Foo& foo )
        : m_foo( foo )
    {
        recalcValue( true );

        m_foo.valueChanged.connect( //
            [this]( const auto& ) { //
                recalcValue( false );
            } );
    }

    [[nodiscard]] int value() const
    {
        return m_value;
    }

    boost::signals2::signal<void( int )> valueChanged;

private:
    void recalcValue( bool initial )
    {
        const int newValue = m_foo.value() + 1;
        if( m_value != newValue )
        {
            m_value = newValue;
            if( !initial )
                valueChanged( newValue );
        }
    }

    Foo& m_foo;
    int m_value = 0;
};

int main()
{
    Foo foo{ 0 };
    Bar bar{ foo };

    // Subscribe to all notifiers to add verbosity
    foo.valueChanged.connect(
        []( int value ) { fmt::print( "foo value is changed to {}\n", value ); } );
    bar.valueChanged.connect(
        []( int value ) { fmt::print( "bar value is changed to {}\n", value ); } );

    auto separator = []() { fmt::print( "=================\n" ); };

    // change variables to see what happens
    foo.setValue( 1 );
    separator();

    foo.setValue( 2 );
    separator();
}
