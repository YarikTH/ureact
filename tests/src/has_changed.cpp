//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/has_changed.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/lift.hpp"
#include "ureact/events.hpp"
#include "ureact/signal.hpp"
#include "ureact/temp_signal.hpp"

namespace
{

// If "has_changed" is not overloaded for type, then it is assumed changed
template <class T>
bool checkIfLackOfChangeDetected( T value = {} )
{
    using ureact::detail::has_changed;
    return !has_changed( value, value );
}

enum E
{
};

enum class Ec
{
};

namespace ns
{

struct ClassWithoutHasChangedOverload
{
    int value;
};

struct ClassWithHasChangedOverload
{
    int value;
};

constexpr bool has_changed(
    const ClassWithHasChangedOverload lhs, const ClassWithHasChangedOverload rhs )
{
    return !( lhs.value == rhs.value );
}

} // namespace ns

} // namespace

TEST_CASE( "HasChanged" )
{
    using ureact::detail::has_changed;

    // integral types
    CHECK( checkIfLackOfChangeDetected<bool>() );
    CHECK( checkIfLackOfChangeDetected<char>() );
    CHECK( checkIfLackOfChangeDetected<signed char>() );
    CHECK( checkIfLackOfChangeDetected<unsigned char>() );
    // CHECK( checkIfLackOfChangeDetected<char8_t>() ); C++20 only
    CHECK( checkIfLackOfChangeDetected<char16_t>() );
    CHECK( checkIfLackOfChangeDetected<char32_t>() );
    CHECK( checkIfLackOfChangeDetected<wchar_t>() );
    CHECK( checkIfLackOfChangeDetected<short>() );
    CHECK( checkIfLackOfChangeDetected<unsigned short>() );
    CHECK( checkIfLackOfChangeDetected<int>() );
    CHECK( checkIfLackOfChangeDetected<unsigned int>() );
    CHECK( checkIfLackOfChangeDetected<long>() );
    CHECK( checkIfLackOfChangeDetected<long long>() );
    CHECK( checkIfLackOfChangeDetected<unsigned long>() );
    CHECK( checkIfLackOfChangeDetected<unsigned long long>() );

    // floating point types
    CHECK( checkIfLackOfChangeDetected<float>() );
    CHECK( checkIfLackOfChangeDetected<double>() );
    CHECK( checkIfLackOfChangeDetected<long double>() );

    // enum types
    CHECK( checkIfLackOfChangeDetected<E>() );
    CHECK( checkIfLackOfChangeDetected<Ec>() );

    // classes
    CHECK_FALSE( checkIfLackOfChangeDetected<ns::ClassWithoutHasChangedOverload>() );
    CHECK( checkIfLackOfChangeDetected<ns::ClassWithHasChangedOverload>() );

    // reference wrappers
    {
        ns::ClassWithoutHasChangedOverload no{ 0 };
        ns::ClassWithHasChangedOverload yes{ 0 };
        CHECK_FALSE( checkIfLackOfChangeDetected( std::ref( no ) ) );
        CHECK( checkIfLackOfChangeDetected( std::ref( yes ) ) );
    }

    // signal
    {
        ureact::context ctx;

        ureact::var_signal v = ureact::make_var( ctx, 0 );
        ureact::signal s = ureact::make_const( ctx, 0.0 );
        ureact::temp_signal t = v + 1;
        CHECK( checkIfLackOfChangeDetected( v ) );
        CHECK( checkIfLackOfChangeDetected( s ) );
        CHECK( checkIfLackOfChangeDetected( t ) );
    }

    // event
    {
        ureact::context ctx;

        ureact::event_source s = ureact::make_source( ctx );
        ureact::events n = ureact::make_never<int>( ctx );
        CHECK( checkIfLackOfChangeDetected( s ) );
        CHECK( checkIfLackOfChangeDetected( n ) );
    }
}
