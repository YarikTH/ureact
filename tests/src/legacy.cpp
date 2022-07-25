//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <string>

#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

namespace
{

class Company
{
public:
    int index;
    ureact::var_signal<std::string> name;

    Company( ureact::context& ctx, const int index, const char* name )
        : index( index )
        , name( make_var( ctx, std::string( name ) ) )
    {}

    friend bool operator==( const Company& lhs, const Company& rhs )
    {
        // clang-format off
        return std::tie( lhs.index, lhs.name.get() )
            == std::tie( rhs.index, rhs.name.get() );
        // clang-format on
    }
};

class Employee
{
public:
    ureact::var_signal<std::reference_wrapper<Company>> company;

    Employee( ureact::context& ctx, Company& companyRef )
        : company( make_var( ctx, std::ref( companyRef ) ) )
    {}
};

class Employee2
{
public:
    ureact::var_signal<Company*> company;

    Employee2( ureact::context& ctx, Company* companyPtr )
        : company( make_var( ctx, companyPtr ) )
    {}
};

} // namespace

TEST_SUITE_BEGIN( "dynamic_signals_test" );

TEST_CASE( "DynamicSignalReferences" )
{
    ureact::context ctx;

    Company company1( ctx, 1, "MetroTec" );
    Company company2( ctx, 2, "ACME" );

    Employee Alice( ctx, company1 );

    ureact::signal<std::string> aliceCompanyName
        = ureact::reactive_ref( Alice.company, &Company::name );

    std::vector<std::string> result;

    observe( aliceCompanyName, [&]( const std::string& name ) { result.push_back( name ); } );

    company1.name <<= std::string( "ModernTec" );
    Alice.company <<= std::ref( company2 );
    company2.name <<= std::string( "A.C.M.E." );

    CHECK( result == std::vector<std::string>{ "ModernTec", "ACME", "A.C.M.E." } );
}

TEST_CASE( "DynamicSignalPointers" )
{
    ureact::context ctx;

    Company company1( ctx, 1, "MetroTec" );
    Company company2( ctx, 2, "ACME" );

    Employee2 Alice( ctx, &company1 );

    ureact::signal<std::string> aliceCompanyName
        = ureact::reactive_ptr( Alice.company, &Company::name );

    std::vector<std::string> result;

    observe( aliceCompanyName, [&]( const std::string& name ) { result.push_back( name ); } );

    company1.name <<= std::string( "ModernTec" );
    Alice.company <<= &company2;
    company2.name <<= std::string( "A.C.M.E." );

    CHECK( result == std::vector<std::string>{ "ModernTec", "ACME", "A.C.M.E." } );
}

TEST_SUITE_END();

#include <iostream>
#include <string>

TEST_SUITE_BEGIN( "ObserverTest" );

TEST_CASE( "detach" )
{
    ureact::context ctx;

    auto a1 = make_var( ctx, 1 );
    auto a2 = make_var( ctx, 1 );

    auto result = a1 + a2;

    int observeCount1 = 0;
    int observeCount2 = 0;
    int observeCount3 = 0;

    int phase = 0;

    auto obs1 = observe( result, [&]( int v ) {
        observeCount1++;

        if( phase == 0 )
            CHECK( v == 3 );
        else
            FAIL( "We shouldn't be here" );
    } );

    auto obs2 = observe( result, [&]( int v ) {
        observeCount2++;

        if( phase == 0 )
            CHECK( v == 3 );
        else if( phase == 1 )
            CHECK( v == 4 );
        else
            FAIL( "We shouldn't be here" );
    } );

    auto obs3 = observe( result, [&]( int v ) {
        observeCount3++;

        if( phase == 0 )
            CHECK( v == 3 );
        else if( phase == 1 )
            CHECK( v == 4 );
        else
            FAIL( "We shouldn't be here" );
    } );

    phase = 0;
    a1 <<= 2;
    CHECK( observeCount1 == 1 );
    CHECK( observeCount2 == 1 );
    CHECK( observeCount3 == 1 );

    phase = 1;
    obs1.detach();
    a1 <<= 3;
    CHECK( observeCount1 == 1 );
    CHECK( observeCount2 == 2 );
    CHECK( observeCount3 == 2 );

    phase = 2;
    obs2.detach();
    obs3.detach();
    a1 <<= 4;
    CHECK( observeCount1 == 1 );
    CHECK( observeCount2 == 2 );
    CHECK( observeCount3 == 2 );
}

TEST_CASE( "NoObserveOnNoChanged" )
{
    ureact::context ctx;

    auto a = make_var( ctx, 1 );
    auto b = make_var( ctx, 1 );

    auto product = a * b;

    auto expressionString
        = lift( with( a, b, product ), []( const int a_, const int b_, const int product_ ) {
              return std::to_string( a_ ) + " * " + std::to_string( b_ ) + " = "
                   + std::to_string( product_ );
          } );

    int aObserveCount = 0;
    int bObserveCount = 0;
    int productObserveCount = 0;

    observe( a, [&]( int /*v*/ ) { ++aObserveCount; } );
    observe( b, [&]( int /*v*/ ) { ++bObserveCount; } );
    observe( product, [&]( int /*v*/ ) { ++productObserveCount; } );

    CHECK( aObserveCount == 0 );
    CHECK( bObserveCount == 0 );
    CHECK( productObserveCount == 0 );
    CHECK( expressionString.get() == "1 * 1 = 1" );

    b <<= 2;
    CHECK( aObserveCount == 0 );
    CHECK( bObserveCount == 1 );
    CHECK( productObserveCount == 1 );
    CHECK( expressionString.get() == "1 * 2 = 2" );

    b <<= 2; // Shouldn't change
    CHECK( aObserveCount == 0 );
    CHECK( bObserveCount == 1 );
    CHECK( productObserveCount == 1 );
    CHECK( expressionString.get() == "1 * 2 = 2" );

    do_transaction( ctx, [&]() {
        b <<= 1;
        b <<= 2; // Shouldn't change
    } );
    CHECK( aObserveCount == 0 );
    CHECK( bObserveCount == 1 );
    CHECK( productObserveCount == 1 );
    CHECK( expressionString.get() == "1 * 2 = 2" );

    a <<= 0;
    CHECK( aObserveCount == 1 );
    CHECK( bObserveCount == 1 );
    CHECK( productObserveCount == 2 );
    CHECK( expressionString.get() == "0 * 2 = 0" );

    b <<= 3;
    CHECK( aObserveCount == 1 );
    CHECK( bObserveCount == 2 );
    CHECK( productObserveCount == 2 ); // Product shouldn't change
    CHECK( expressionString.get() == "0 * 3 = 0" );
}

TEST_CASE( "ScopedObserverTest" )
{
    ureact::context ctx;

    std::vector<int> results;

    auto in = make_var( ctx, 1 );

    {
        ureact::scoped_observer obs = observe( in, [&]( int v ) { results.push_back( v ); } );

        in <<= 2;
    }

    in <<= 3;

    CHECK( results.size() == 1 );
    CHECK( results[0] == 2 );
}

TEST_CASE( "SelfObserverDetachTest" )
{
    ureact::context ctx;

    std::vector<int> results;

    auto in = make_var( ctx, 0 );

    ureact::observer obs = observe( in, [&]( int v ) {
        if( v < 0 )
        {
            return ureact::observer_action::stop_and_detach;
        }
        else
        {
            results.push_back( v );
            return ureact::observer_action::next;
        }
    } );

    in <<= 1;
    in <<= 2;
    in <<= -1;
    in <<= 3;

    CHECK( results.size() == 2 );
    CHECK( results[0] == 1 );
    CHECK( results[1] == 2 );
}

TEST_SUITE_END();

namespace
{
using namespace ureact;

} // namespace

TEST_SUITE( "OperationsTest" )
{
    TEST_CASE( "detach" )
    {
        context ctx;

        auto a1 = make_var( ctx, 1 );
        auto a2 = make_var( ctx, 1 );

        auto result = lift( with( a1, a2 ), []( int a, int b ) { return a + b; } );

        int observeCount1 = 0;
        int observeCount2 = 0;
        int observeCount3 = 0;

        int phase;

        auto obs1 = observe( result, [&]( int v ) {
            observeCount1++;

            if( phase == 0 )
                CHECK_EQ( v, 3 );
            else if( phase == 1 )
                CHECK_EQ( v, 4 );
            else
                CHECK( false );
        } );

        auto obs2 = observe( result, [&]( int v ) {
            observeCount2++;

            if( phase == 0 )
                CHECK_EQ( v, 3 );
            else if( phase == 1 )
                CHECK_EQ( v, 4 );
            else
                CHECK( false );
        } );

        auto obs3 = observe( result, [&]( int v ) {
            observeCount3++;

            if( phase == 0 )
                CHECK_EQ( v, 3 );
            else if( phase == 1 )
                CHECK_EQ( v, 4 );
            else
                CHECK( false );
        } );

        phase = 0;
        a1 <<= 2;
        CHECK_EQ( observeCount1, 1 );
        CHECK_EQ( observeCount2, 1 );
        CHECK_EQ( observeCount3, 1 );

        phase = 1;
        obs1.detach();
        a1 <<= 3;
        CHECK_EQ( observeCount1, 1 );
        CHECK_EQ( observeCount2, 2 );
        CHECK_EQ( observeCount3, 2 );

        phase = 2;
        obs2.detach();
        obs3.detach();
        a1 <<= 4;
        CHECK_EQ( observeCount1, 1 );
        CHECK_EQ( observeCount2, 2 );
        CHECK_EQ( observeCount3, 2 );
    }

    TEST_CASE( "ScopedObserverTest" )
    {
        std::vector<int> results;

        context ctx;

        auto in = make_var( ctx, 1 );

        {
            scoped_observer obs = observe( in, [&]( int v ) { results.push_back( v ); } );

            in <<= 2;
        }

        in <<= 3;

        CHECK_EQ( results.size(), 1 );
        CHECK_EQ( results[0], 2 );
    }

    TEST_CASE( "SyncedObserveTest" )
    {
        context ctx;

        auto in1 = make_var( ctx, 1 );
        auto in2 = make_var( ctx, 1 );

        auto sum = lift( with( in1, in2 ), []( int a, int b ) { return a + b; } );
        auto prod = lift( with( in1, in2 ), []( int a, int b ) { return a * b; } );
        auto diff = lift( with( in1, in2 ), []( int a, int b ) { return a - b; } );

        auto src1 = make_source( ctx );
        auto src2 = make_source<int>( ctx );

        observe( src1, with( sum, prod, diff ), []( unit, int sum, int prod, int diff ) {
            CHECK_EQ( sum, 33 );
            CHECK_EQ( prod, 242 );
            CHECK_EQ( diff, 11 );
        } );

        observe( src2, with( sum, prod, diff ), []( int e, int sum, int prod, int diff ) {
            CHECK_EQ( e, 42 );
            CHECK_EQ( sum, 33 );
            CHECK_EQ( prod, 242 );
            CHECK_EQ( diff, 11 );
        } );

        in1 <<= 22;
        in2 <<= 11;

        src1.emit();
        src2.emit( 42 );
    }

    TEST_CASE( "DetachThisObserver1" )
    {
        context ctx;

        auto src = make_source( ctx );

        int count = 0;

        observe( src, [&]( unit ) -> observer_action {
            ++count;
            if( count == 1 )
            {
                return observer_action::next;
            }
            return observer_action::stop_and_detach;
        } );

        src.emit();
        src.emit();
        src.emit();
        src.emit();

        CHECK_EQ( count, 2 );
    }

    TEST_CASE( "DetachThisObserver2" )
    {
        context ctx;

        auto in1 = make_var( ctx, 1 );
        auto in2 = make_var( ctx, 1 );

        auto sum = lift( with( in1, in2 ), []( int a, int b ) { return a + b; } );
        auto prod = lift( with( in1, in2 ), []( int a, int b ) { return a * b; } );
        auto diff = lift( with( in1, in2 ), []( int a, int b ) { return a - b; } );

        auto src = make_source( ctx );

        int count = 0;

        observe( src,
            with( sum, prod, diff ),
            [&]( unit, int sum, int prod, int diff ) -> observer_action {
                ++count;
                if( count == 1 )
                {
                    return observer_action::next;
                }
                return observer_action::stop_and_detach;
            } );

        in1 <<= 22;
        in2 <<= 11;

        src.emit();
        src.emit();
        src.emit();
        src.emit();

        CHECK_EQ( count, 2 );
    }

    TEST_CASE( "Detaching observers using return value" )
    {
        context ctx;

        auto x = make_var( ctx, 0 );

        std::vector<int> x_values;

        // Functor used for observer can optionally return value
        // Using this value observer can be optionally self detached
        auto obs = observe( x, [&]( const int v ) {
            if( v < 0 )
            {
                return observer_action::stop_and_detach;
            }
            else
            {
                x_values.push_back( v );
                return observer_action::next;
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

#include <sstream>
#include <string>

namespace
{

enum class binary_operator_type
{
    signal_op_signal,
    temp_signal_op_signal,
    signal_op_temp_signal,
    temp_signal_op_temp_signal,
    value_op_signal,
    value_op_temp_signal,
    signal_op_value,
    temp_signal_op_value
};

#define MAKE_BINARY_OPERATOR_SIGNAL( OPERATOR, NAME )                                              \
    template <typename left_t,                                                                     \
        typename right_t,                                                                          \
        typename ret_t = decltype( std::declval<left_t>() OPERATOR std::declval<right_t>() )>      \
    auto make_binary_operator_signal_##NAME(                                                       \
        ureact::context& ctx, binary_operator_type type, const left_t& lhs, const right_t& rhs )   \
        ->ureact::signal<ret_t>                                                                    \
    {                                                                                              \
        switch( type )                                                                             \
        {                                                                                          \
            case binary_operator_type::signal_op_signal:                                           \
                return make_var( ctx, lhs ) OPERATOR make_var( ctx, rhs );                         \
            case binary_operator_type::temp_signal_op_signal:                                      \
                return (+make_var( ctx, lhs ))OPERATOR make_var( ctx, rhs );                       \
            case binary_operator_type::signal_op_temp_signal:                                      \
                return make_var( ctx, lhs ) OPERATOR( +make_var( ctx, rhs ) );                     \
            case binary_operator_type::temp_signal_op_temp_signal:                                 \
                return (+make_var( ctx, lhs ))OPERATOR( +make_var( ctx, rhs ) );                   \
            case binary_operator_type::value_op_signal: return lhs OPERATOR make_var( ctx, rhs );  \
            case binary_operator_type::value_op_temp_signal:                                       \
                return lhs OPERATOR( +make_var( ctx, rhs ) );                                      \
            case binary_operator_type::signal_op_value: return make_var( ctx, lhs ) OPERATOR rhs;  \
            case binary_operator_type::temp_signal_op_value:                                       \
                return (+make_var( ctx, lhs ))OPERATOR rhs;                                        \
        }                                                                                          \
        return {};                                                                                 \
    }

#define BINARY_OPERATOR( OPERATOR, NAME )                                                          \
    template <typename left_t,                                                                     \
        typename right_t,                                                                          \
        typename ret_t = decltype( std::declval<left_t>() OPERATOR std::declval<right_t>() )>      \
    auto binary_operator_##NAME( const left_t& lhs, const right_t& rhs )->ret_t                    \
    {                                                                                              \
        return lhs OPERATOR rhs;                                                                   \
    }

MAKE_BINARY_OPERATOR_SIGNAL( +, addition )
BINARY_OPERATOR( +, addition )

#define DECLARE_BINARY_OPERATOR_TRAITS( OPERATOR, NAME )                                           \
    struct binary_operator_traits_##NAME                                                           \
    {                                                                                              \
        static const char* get_name()                                                              \
        {                                                                                          \
            return #NAME;                                                                          \
        }                                                                                          \
        static const char* get_operator()                                                          \
        {                                                                                          \
            return #OPERATOR;                                                                      \
        }                                                                                          \
                                                                                                   \
        template <typename left_t, typename right_t>                                               \
        static auto lift( ureact::context& ctx,                                                    \
            const binary_operator_type type,                                                       \
            const left_t& lhs,                                                                     \
            const right_t& rhs )                                                                   \
            -> ureact::signal<typename decltype( std::declval<ureact::signal<left_t>>()            \
                    OPERATOR std::declval<ureact::signal<right_t>>() )::value_t>                   \
        {                                                                                          \
            switch( type )                                                                         \
            {                                                                                      \
                case binary_operator_type::signal_op_signal:                                       \
                    return make_var( ctx, lhs ) OPERATOR make_var( ctx, rhs );                     \
                case binary_operator_type::temp_signal_op_signal:                                  \
                    return (+make_var( ctx, lhs ))OPERATOR make_var( ctx, rhs );                   \
                case binary_operator_type::signal_op_temp_signal:                                  \
                    return make_var( ctx, lhs ) OPERATOR( +make_var( ctx, rhs ) );                 \
                case binary_operator_type::temp_signal_op_temp_signal:                             \
                    return (+make_var( ctx, lhs ))OPERATOR( +make_var( ctx, rhs ) );               \
                case binary_operator_type::value_op_signal:                                        \
                    return lhs OPERATOR make_var( ctx, rhs );                                      \
                case binary_operator_type::value_op_temp_signal:                                   \
                    return lhs OPERATOR( +make_var( ctx, rhs ) );                                  \
                case binary_operator_type::signal_op_value:                                        \
                    return make_var( ctx, lhs ) OPERATOR rhs;                                      \
                case binary_operator_type::temp_signal_op_value:                                   \
                    return (+make_var( ctx, lhs ))OPERATOR rhs;                                    \
            }                                                                                      \
            return {};                                                                             \
        }                                                                                          \
                                                                                                   \
        template <typename left_t, typename right_t>                                               \
        static auto execute_operator( const left_t& lhs, const right_t& rhs )                      \
            -> decltype( std::declval<left_t>() OPERATOR std::declval<right_t>() )                 \
        {                                                                                          \
            return lhs OPERATOR rhs;                                                               \
        }                                                                                          \
                                                                                                   \
        template <typename left_t, typename right_t>                                               \
        static std::string get_test_name(                                                          \
            const binary_operator_type type, const left_t& lhs, const right_t& rhs )               \
        {                                                                                          \
            std::ostringstream ss;                                                                 \
            switch( type )                                                                         \
            {                                                                                      \
                case binary_operator_type::signal_op_signal:                                       \
                    ss << "signal(" << lhs << ") " #OPERATOR " signal(" << rhs << ")";             \
                    break;                                                                         \
                case binary_operator_type::temp_signal_op_signal:                                  \
                    ss << "temp_signal(" << lhs << ") " #OPERATOR " signal(" << rhs << ")";        \
                    break;                                                                         \
                case binary_operator_type::signal_op_temp_signal:                                  \
                    ss << "signal(" << lhs << ") " #OPERATOR " temp_signal(" << rhs << ")";        \
                    break;                                                                         \
                case binary_operator_type::temp_signal_op_temp_signal:                             \
                    ss << "temp_signal(" << lhs << ") " #OPERATOR " temp_signal(" << rhs << ")";   \
                    break;                                                                         \
                case binary_operator_type::value_op_signal:                                        \
                    ss << "value(" << lhs << ") " #OPERATOR " signal(" << rhs << ")";              \
                    break;                                                                         \
                case binary_operator_type::value_op_temp_signal:                                   \
                    ss << "value(" << lhs << ") " #OPERATOR " temp_signal(" << rhs << ")";         \
                    break;                                                                         \
                case binary_operator_type::signal_op_value:                                        \
                    ss << "signal(" << lhs << ") " #OPERATOR " value(" << rhs << ")";              \
                    break;                                                                         \
                case binary_operator_type::temp_signal_op_value:                                   \
                    ss << "temp_signal(" << lhs << ") " #OPERATOR " value(" << rhs << ")";         \
                    break;                                                                         \
            }                                                                                      \
            return ss.str();                                                                       \
        }                                                                                          \
    };

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wunknown-warning-option"
#    pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#endif

DECLARE_BINARY_OPERATOR_TRAITS( +, addition )
DECLARE_BINARY_OPERATOR_TRAITS( -, subtraction )
DECLARE_BINARY_OPERATOR_TRAITS( *, multiplication )
DECLARE_BINARY_OPERATOR_TRAITS( /, division )
DECLARE_BINARY_OPERATOR_TRAITS( %, modulo )

DECLARE_BINARY_OPERATOR_TRAITS( ==, equal )
DECLARE_BINARY_OPERATOR_TRAITS( !=, not_equal )
DECLARE_BINARY_OPERATOR_TRAITS( <, less )
DECLARE_BINARY_OPERATOR_TRAITS( <=, less_equal )
DECLARE_BINARY_OPERATOR_TRAITS( >, greater )
DECLARE_BINARY_OPERATOR_TRAITS( >=, greater_equal )

DECLARE_BINARY_OPERATOR_TRAITS( &&, logical_and )
DECLARE_BINARY_OPERATOR_TRAITS( ||, logical_or )

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic pop
#endif

template <typename traits, typename left_t, typename right_t>
void test_binary_operator_impl( binary_operator_type type, const left_t& lhs, const right_t& rhs )
{
    ureact::context ctx;
    auto signal_to_test = traits::lift( ctx, type, lhs, rhs );
    auto value_from_signal = signal_to_test.get();
    auto value_from_operator = traits::execute_operator( lhs, rhs );
    static_assert(
        std::is_same<decltype( value_from_signal ), decltype( value_from_operator )>::value,
        "Value type from signal should match return type of operator" );
    CHECK( value_from_signal == value_from_operator );
}

template <typename traits, typename left_t, typename right_t>
void test_binary_operator( const left_t& lhs, const right_t& rhs )
{
    for( auto type : { binary_operator_type::signal_op_signal,
             binary_operator_type::temp_signal_op_signal,
             binary_operator_type::signal_op_temp_signal,
             binary_operator_type::temp_signal_op_temp_signal,
             binary_operator_type::value_op_signal,
             binary_operator_type::value_op_temp_signal,
             binary_operator_type::signal_op_value,
             binary_operator_type::temp_signal_op_value } )
    {
        SUBCASE( traits::get_test_name( type, lhs, rhs ).c_str() )
        {
            test_binary_operator_impl<traits>( type, lhs, rhs );
        }
    }
}

} // namespace

TEST_SUITE( "operators" )
{
    TEST_CASE( "unary operators" )
    {
        ureact::context ctx;

        auto v1 = make_var( ctx, 1 );

        // clang-format off
        auto unary_plus           = +v1;
        auto unary_minus          = -v1;
        auto logical_negation     = !v1;
        auto unary_plus_2         = +(+v1);
        auto unary_minus_2        = -(+v1);
        auto logical_negation_2   = !(+v1);
        // clang-format on

        auto checkValues = [&]( std::initializer_list<int> valuesToTest ) {
            for( const int& value : valuesToTest )
            {
                v1 <<= value;

                // clang-format off
                CHECK( unary_plus.get()           == (+value) );
                CHECK( unary_minus.get()          == (-value) );
                CHECK( logical_negation.get()     == (!value) );
                CHECK( unary_plus_2.get()         == (+value) );
                CHECK( unary_minus_2.get()        == (-value) );
                CHECK( logical_negation_2.get()   == (!value) );
                // clang-format on
            }
        };

        checkValues( { 0, 1, -4, 654 } );
    }

    TEST_CASE( "binary operators (normal)" )
    {
        ureact::context ctx;

        auto lhs = make_var( ctx, 0 );
        auto rhs = make_var( ctx, 1 );

        // clang-format off
        auto addition            = lhs +  rhs;
        auto subtraction         = lhs -  rhs;
        auto multiplication      = lhs *  rhs;
        auto equal               = lhs == rhs;
        auto not_equal           = lhs != rhs;
        auto less                = lhs <  rhs;
        auto less_equal          = lhs <= rhs;
        auto greater             = lhs >  rhs;
        auto greater_equal       = lhs >= rhs;
        auto logical_and         = lhs && rhs;
        auto logical_or          = lhs || rhs;
        // clang-format on

        std::initializer_list<std::pair<int, int>> values_to_test = {
            { 2, 2 },
            { 3, -3 },
            { 0, 0 },
            { -4, 7 },
            { -8, -2 },
            { 0, -2 },
            { 5, 0 },
        };

        for( const auto& values : values_to_test )
        {
            int left, right;
            std::tie( left, right ) = values;

            SUBCASE( ( std::to_string( left ) + std::string( " op " ) + std::to_string( right ) )
                         .c_str() )
            {
                do_transaction( ctx, [&]() {
                    lhs <<= left;
                    rhs <<= right;
                } );

                // clang-format off
                CHECK( addition.get()            == (left +  right) );
                CHECK( subtraction.get()         == (left -  right) );
                CHECK( multiplication.get()      == (left *  right) );
                CHECK( equal.get()               == (left == right) );
                CHECK( not_equal.get()           == (left != right) );
                CHECK( less.get()                == (left <  right) );
                CHECK( less_equal.get()          == (left <= right) );
                CHECK( greater.get()             == (left >  right) );
                CHECK( greater_equal.get()       == (left >= right) );
                CHECK( logical_and.get()         == (left && right) );
                CHECK( logical_or.get()          == (left || right) );
                // clang-format on
            }
        }
    }

    TEST_CASE( "binary operators (divisible)" )
    {
        ureact::context ctx;

        auto lhs = make_var( ctx, 0 );
        auto rhs = make_var( ctx, 1 );

        // clang-format off
        auto division = lhs / rhs;
        auto modulo   = lhs % rhs;
        // clang-format on

        std::initializer_list<std::pair<int, int>> values_to_test = {
            { 2, 2 },
            { 3, -3 },
            { 8, 3 },
        };

        for( const auto& values : values_to_test )
        {
            int left, right;
            std::tie( left, right ) = values;
            SUBCASE( ( std::to_string( left ) + std::string( " op " ) + std::to_string( right ) )
                         .c_str() )
            {
                do_transaction( ctx, [&]() {
                    lhs <<= left;
                    rhs <<= right;
                } );
                // clang-format off
                CHECK( division.get() == (left / right) );
                CHECK( modulo.get()   == (left % right) );
                // clang-format on
            }
        }
    }

    TEST_CASE( "binary operators (full test)" )
    {
        test_binary_operator<binary_operator_traits_addition>( 2, 2 );
        test_binary_operator<binary_operator_traits_addition>( 3, -3 );
        test_binary_operator<binary_operator_traits_addition>( 1, 0.1f );
        test_binary_operator<binary_operator_traits_addition>( true, 0 );

        test_binary_operator<binary_operator_traits_division>( 2, 2 );
        test_binary_operator<binary_operator_traits_division>( 3, -3 );
        test_binary_operator<binary_operator_traits_division>( 8, 3 );
        test_binary_operator<binary_operator_traits_division>( 0, 1 );
        test_binary_operator<binary_operator_traits_division>( 10, 3.0 );

        test_binary_operator<binary_operator_traits_modulo>( 2, 2 );
        test_binary_operator<binary_operator_traits_modulo>( 3, -3 );
        test_binary_operator<binary_operator_traits_modulo>( 8, 3 );
        test_binary_operator<binary_operator_traits_modulo>( 0, 1 );
    }

    TEST_CASE( "priority" )
    {
        // https://en.cppreference.com/w/cpp/language/operator_precedence
        // 3.  +a -a          Unary plus and minus
        //     ! ~            Logical NOT and bitwise NOT
        // 5.  a*b  a/b  a%b  Multiplication, division, and remainder
        // 6.  a+b  a-b       Addition and subtraction
        // 8.  <=>            Three-way comparison operator (since C++20)
        // 9.  <  <=          For relational operators < and ≤ respectively
        //     >  >=          For relational operators > and ≥ respectively
        // 10. ==  !=         For equality operators = and ≠ respectively
        // 14. &&             Logical AND
        // 15. ||             Logical OR

        ureact::context ctx;

        auto _2 = make_var( ctx, 2 );

        auto result = _2 + _2 * _2;
        CHECK( result.get() == 6 );

        auto result2 = ( _2 + _2 ) * _2;
        CHECK( result2.get() == 8 );
    }

} // TEST_SUITE_END

#include <algorithm>
#include <queue>

namespace
{

int add( int a, int b )
{
    return a + b;
}
float halve( int a )
{
    return static_cast<float>( a ) / 2.0f;
}
float multiply( float a, float b )
{
    return a * b;
}

} // namespace

TEST_SUITE_BEGIN( "SignalTest" );

TEST_CASE( "MakeVars" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 1 );
    auto v2 = make_var( ctx, 2 );
    auto v3 = make_var( ctx, 3 );
    auto v4 = make_var( ctx, 4 );

    CHECK( v1.get() == 1 );
    CHECK( v2.get() == 2 );
    CHECK( v3.get() == 3 );
    CHECK( v4.get() == 4 );

    v1 <<= 10;
    v2 <<= 20;
    v3 <<= 30;
    v4 <<= 40;

    CHECK( v1.get() == 10 );
    CHECK( v2.get() == 20 );
    CHECK( v3.get() == 30 );
    CHECK( v4.get() == 40 );
}

TEST_CASE( "Signals1" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 1 );
    auto v2 = make_var( ctx, 2 );
    auto v3 = make_var( ctx, 3 );
    auto v4 = make_var( ctx, 4 );

    auto s1 = lift( with( v1, v2 ), []( int a, int b ) { return a + b; } );

    auto s2 = lift( with( v3, v4 ), []( int a, int b ) { return a + b; } );

    auto s3 = s1 + s2;

    CHECK( s1.get() == 3 );
    CHECK( s2.get() == 7 );
    CHECK( s3.get() == 10 );

    v1 <<= 10;
    v2 <<= 20;
    v3 <<= 30;
    v4 <<= 40;

    CHECK( s1.get() == 30 );
    CHECK( s2.get() == 70 );
    CHECK( s3.get() == 100 );

    CHECK( ureact::is_signal<decltype( v1 )>::value );

    CHECK( ureact::is_signal<decltype( s1 )>::value );

    CHECK( ureact::is_signal<decltype( s2 )>::value );

    CHECK_FALSE( ureact::is_signal<decltype( 10 )>::value );
}

TEST_CASE( "Signals2" )
{
    ureact::context ctx;

    auto a1 = make_var( ctx, 1 );
    auto a2 = make_var( ctx, 1 );

    auto b1 = a1 + 0;
    auto b2 = a1 + a2;
    auto b3 = a2 + 0;

    auto c1 = b1 + b2;
    auto c2 = b2 + b3;

    auto result = c1 + c2;

    int observeCount = 0;

    observe( result, [&observeCount]( int v ) {
        observeCount++;
        if( observeCount == 1 )
            CHECK( v == 9 );
        else
            CHECK( v == 12 );
    } );

    CHECK( a1.get() == 1 );
    CHECK( a2.get() == 1 );

    CHECK( b1.get() == 1 );
    CHECK( b2.get() == 2 );
    CHECK( b3.get() == 1 );

    CHECK( c1.get() == 3 );
    CHECK( c2.get() == 3 );

    CHECK( result.get() == 6 );

    a1 <<= 2;

    CHECK( observeCount == 1 );

    CHECK( a1.get() == 2 );
    CHECK( a2.get() == 1 );

    CHECK( b1.get() == 2 );
    CHECK( b2.get() == 3 );
    CHECK( b3.get() == 1 );

    CHECK( c1.get() == 5 );
    CHECK( c2.get() == 4 );

    CHECK( result.get() == 9 );

    a2 <<= 2;

    CHECK( observeCount == 2 );

    CHECK( a1.get() == 2 );
    CHECK( a2.get() == 2 );

    CHECK( b1.get() == 2 );
    CHECK( b2.get() == 4 );
    CHECK( b3.get() == 2 );

    CHECK( c1.get() == 6 );
    CHECK( c2.get() == 6 );

    CHECK( result.get() == 12 );
}

TEST_CASE( "Signals3" )
{
    ureact::context ctx;

    auto a1 = make_var( ctx, 1 );
    auto a2 = make_var( ctx, 1 );

    auto b1 = a1 + 0;
    auto b2 = a1 + a2;
    auto b3 = a2 + 0;

    auto c1 = b1 + b2;
    auto c2 = b2 + b3;

    auto result = c1 + c2;

    int observeCount = 0;

    observe( result, [&observeCount]( int v ) {
        observeCount++;
        CHECK( v == 12 );
    } );

    CHECK( a1.get() == 1 );
    CHECK( a2.get() == 1 );

    CHECK( b1.get() == 1 );
    CHECK( b2.get() == 2 );
    CHECK( b3.get() == 1 );

    CHECK( c1.get() == 3 );
    CHECK( c2.get() == 3 );

    CHECK( result.get() == 6 );

    do_transaction( ctx, [&] {
        a1 <<= 2;
        a2 <<= 2;
    } );

    CHECK( observeCount == 1 );

    CHECK( a1.get() == 2 );
    CHECK( a2.get() == 2 );

    CHECK( b1.get() == 2 );
    CHECK( b2.get() == 4 );
    CHECK( b3.get() == 2 );

    CHECK( c1.get() == 6 );
    CHECK( c2.get() == 6 );

    CHECK( result.get() == 12 );
}

TEST_CASE( "Signals4" )
{
    ureact::context ctx;

    auto a1 = make_var( ctx, 1 );
    auto a2 = make_var( ctx, 1 );

    auto b1 = a1 + a2;
    auto b2 = b1 + a2;

    CHECK( a1.get() == 1 );
    CHECK( a2.get() == 1 );

    CHECK( b1.get() == 2 );
    CHECK( b2.get() == 3 );

    a1 <<= 10;

    CHECK( a1.get() == 10 );
    CHECK( a2.get() == 1 );

    CHECK( b1.get() == 11 );
    CHECK( b2.get() == 12 );
}

TEST_CASE( "FunctionBind1" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 2 );
    auto v2 = make_var( ctx, 30 );
    auto v3 = make_var( ctx, 10 );

    auto signal = with( v1, v2, v3 )
                | ureact::lift( [=]( int a, int b, int c ) -> int { return a * b * c; } );

    CHECK( signal.get() == 600 );
    v3 <<= 100;
    CHECK( signal.get() == 6000 );
}

TEST_CASE( "FunctionBind2" )
{
    ureact::context ctx;

    auto a = make_var( ctx, 1 );
    auto b = make_var( ctx, 1 );

    auto c = with( a + b, a + 100 ) | ureact::lift( &add );
    auto d = c | ureact::lift( &halve );
    auto e = with( d, d ) | ureact::lift( &multiply );
    auto f = -e + 100.f;

    CHECK( c.get() == 103 );
    CHECK( d.get() == 51.5f );
    CHECK( e.get() == 2652.25f );
    CHECK( f.get() == -2552.25f );

    a <<= 10;

    CHECK( c.get() == 121 );
    CHECK( d.get() == 60.5f );
    CHECK( e.get() == 3660.25f );
    CHECK( f.get() == -3560.25f );
}

TEST_CASE( "Compose signals" )
{
    ureact::context ctx;

    auto a = make_var( ctx, 1 );
    auto b = make_var( ctx, 1 );
    auto inverse_value = ureact::lift( []( int i ) { return -i; } );
    auto double_value = ureact::lift( []( int i ) { return i * 2; } );
    auto sum_values = ureact::lift( []( int i, int j ) { return i + j; } );

    // x = a * 2 * 2 * 2 == a * 8
    auto x = a | double_value | double_value | double_value;

    // y = (2 * a + (-1) * b) * 2
    auto y = with( a | double_value, b | inverse_value ) | sum_values | double_value;

    CHECK( x.get() == 1 * 2 * 2 * 2 );
    CHECK( y.get() == ( 2 * 1 + ( -1 ) * 1 ) * 2 );

    a <<= 7;
    b <<= 5;

    CHECK( x.get() == 7 * 2 * 2 * 2 );
    CHECK( y.get() == ( 2 * 7 + ( -1 ) * 5 ) * 2 );
}

TEST_CASE( "Flatten1" )
{
    ureact::context ctx;

    auto inner1 = make_var( ctx, 123 );
    auto inner2 = make_var( ctx, 789 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = flatten( outer );

    std::queue<int> results;

    observe( flattened, [&]( int v ) { results.push( v ); } );

    CHECK( outer.get().equals( inner1 ) );
    CHECK( flattened.get() == 123 );

    inner1 <<= 456;

    CHECK( flattened.get() == 456 );

    CHECK( results.front() == 456 );
    results.pop();
    CHECK( results.empty() );

    outer <<= inner2;

    CHECK( outer.get().equals( inner2 ) );
    CHECK( flattened.get() == 789 );

    CHECK( results.front() == 789 );
    results.pop();
    CHECK( results.empty() );
}

TEST_CASE( "Flatten2" )
{
    ureact::context ctx;

    auto a0 = make_var( ctx, 100 );

    auto inner1 = make_var( ctx, 200 );

    auto a1 = make_var( ctx, 300 );
    auto a2 = a1 + 0;
    auto a3 = a2 + 0;
    auto a4 = a3 + 0;
    auto a5 = a4 + 0;
    auto a6 = a5 + 0;
    auto inner2 = a6 + 0;

    CHECK( inner1.get() == 200 );
    CHECK( inner2.get() == 300 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = flatten( outer );

    CHECK( flattened.get() == 200 );

    int observeCount = 0;

    observe( flattened, [&observeCount]( int /*unused*/ ) { observeCount++; } );

    auto o1 = a0 + flattened;
    auto o2 = o1 + 0;
    auto o3 = o2 + 0;
    auto result = o3 + 0;

    CHECK( result.get() == 100 + 200 );

    inner1 <<= 1234;

    CHECK( result.get() == 100 + 1234 );
    CHECK( observeCount == 1 );

    outer <<= inner2;

    CHECK( result.get() == 100 + 300 );
    CHECK( observeCount == 2 );

    do_transaction( ctx, [&] {
        a0 <<= 5000;
        a1 <<= 6000;
    } );

    CHECK( result.get() == 5000 + 6000 );
    CHECK( observeCount == 3 );
}

TEST_CASE( "Flatten3" )
{
    ureact::context ctx;

    auto inner1 = make_var( ctx, 10 );

    auto a1 = make_var( ctx, 20 );
    auto a2 = a1 + 0;
    auto a3 = a2 + 0;
    auto inner2 = a3 + 0;

    auto outer = make_var( ctx, inner1 );

    auto a0 = make_var( ctx, 30 );

    auto flattened = flatten( outer );

    int observeCount = 0;

    observe( flattened, [&observeCount]( int /*unused*/ ) { observeCount++; } );

    auto result = flattened + a0;

    CHECK( result.get() == 10 + 30 );
    CHECK( observeCount == 0 );

    do_transaction( ctx, [&] {
        inner1 <<= 1000;
        a0 <<= 200000;
        a1 <<= 50000;
        outer <<= inner2;
    } );

    CHECK( result.get() == 50000 + 200000 );
    CHECK( observeCount == 1 );

    do_transaction( ctx, [&] {
        a0 <<= 667;
        a1 <<= 776;
    } );

    CHECK( result.get() == 776 + 667 );
    CHECK( observeCount == 2 );

    do_transaction( ctx, [&] {
        inner1 <<= 999;
        a0 <<= 888;
    } );

    CHECK( result.get() == 776 + 888 );
    CHECK( observeCount == 2 );
}

TEST_CASE( "Flatten4" )
{
    ureact::context ctx;

    std::vector<int> results;

    auto a1 = make_var( ctx, 100 );
    auto inner1 = a1 + 0;

    auto a2 = make_var( ctx, 200 );
    auto inner2 = a2;

    auto a3 = make_var( ctx, 200 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = flatten( outer );

    auto result = flattened + a3;

    observe( result, [&]( int v ) { results.push_back( v ); } );

    do_transaction( ctx, [&] {
        a3 <<= 400;
        outer <<= inner2;
    } );

    CHECK( results.size() == 1 );

    CHECK( std::find( results.begin(), results.end(), 600 ) != results.end() );
}

TEST_CASE( "Flatten5" )
{
    ureact::context ctx;

    auto inner1 = make_var( ctx, 123 );
    auto inner2 = make_var( ctx, 123 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = flatten( outer );

    std::queue<int> results;

    observe( flattened, [&]( int v ) { results.push( v ); } );

    CHECK( outer.get().equals( inner1 ) );
    CHECK( flattened.get() == 123 );

    CHECK( results.empty() );

    outer <<= inner2;

    CHECK( outer.get().equals( inner2 ) );
    CHECK( flattened.get() == 123 );

    // flattened observer shouldn't trigger if value isn't changed
    CHECK( results.empty() );
}

TEST_CASE( "Modify1" )
{
    ureact::context ctx;

    auto v = make_var( ctx, std::vector<int>{} );

    int obsCount = 0;

    observe( v, [&]( const std::vector<int>& v_ ) {
        REQUIRE( v_.size() == 3 );
        CHECK( v_[0] == 30 );
        CHECK( v_[1] == 50 );
        CHECK( v_[2] == 70 );

        obsCount++;
    } );

    v.modify( []( std::vector<int>& v_ ) {
        v_.push_back( 30 );
        v_.push_back( 50 );
        v_.push_back( 70 );
    } );

    CHECK( obsCount == 1 );
}

TEST_CASE( "Modify2" )
{
    ureact::context ctx;

    auto v = make_var( ctx, std::vector<int>{} );

    int obsCount = 0;

    observe( v, [&]( const std::vector<int>& v_ ) {
        REQUIRE( v_.size() == 3 );
        CHECK( v_[0] == 30 );
        CHECK( v_[1] == 50 );
        CHECK( v_[2] == 70 );

        obsCount++;
    } );

    do_transaction( ctx, [&] {
        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 30 ); } );

        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 50 ); } );

        v.modify( []( std::vector<int>& v_ ) { v_.push_back( 70 ); } );
    } );

    CHECK( obsCount == 1 );
}

TEST_CASE( "Modify3" )
{
    ureact::context ctx;

    auto value = make_var( ctx, std::vector<int>{} );

    int obsCount = 0;

    observe( value, [&]( const std::vector<int>& v ) {
        CHECK( v[0] == 30 );
        CHECK( v[1] == 50 );
        CHECK( v[2] == 70 );

        obsCount++;
    } );

    do_transaction( ctx, [&] {
        value.set( std::vector<int>{ 30, 50 } );

        value.modify( []( std::vector<int>& v ) { v.push_back( 70 ); } );
    } );

    CHECK( obsCount == 1 );
}

TEST_CASE( "Recursive transactions" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 1 );

    int observeCount = 0;

    observe( v1, [&observeCount]( int /*v*/ ) { observeCount++; } );

    do_transaction( ctx, [&] {
        v1 <<= 7;

        do_transaction( ctx, [&] { v1 <<= 4; } );

        v1 <<= 1;
        v1 <<= 2;
    } );

    CHECK( observeCount == 1 );
}

TEST_CASE( "Functional get" )
{
    ureact::context ctx;

    auto v1 = make_var( ctx, 1 );

    CHECK( v1() == 1 );
}

TEST_SUITE_END();
#include <algorithm>
#include <queue>

namespace
{
using namespace ureact;

int myfunc( int a, int b )
{
    return a + b;
}

float myfunc2( int a )
{
    return static_cast<float>( a ) / 2.0f;
}

float myfunc3( float a, float b )
{
    return a * b;
}

} // namespace

TEST_SUITE( "SignalTest" )
{
    TEST_CASE( "MakeVars" )
    {
        context ctx;

        auto v1 = make_var( ctx, 1 );
        auto v2 = make_var( ctx, 2 );
        auto v3 = make_var( ctx, 3 );
        auto v4 = make_var( ctx, 4 );

        CHECK_EQ( v1.get(), 1 );
        CHECK_EQ( v2.get(), 2 );
        CHECK_EQ( v3.get(), 3 );
        CHECK_EQ( v4.get(), 4 );

        v1 <<= 10;
        v2 <<= 20;
        v3 <<= 30;
        v4 <<= 40;

        CHECK_EQ( v1.get(), 10 );
        CHECK_EQ( v2.get(), 20 );
        CHECK_EQ( v3.get(), 30 );
        CHECK_EQ( v4.get(), 40 );
    }

    TEST_CASE( "Signals1" )
    {
        auto summ = ureact::lift( []( int a, int b ) { return a + b; } );

        context ctx;

        auto v1 = make_var( ctx, 1 );
        auto v2 = make_var( ctx, 2 );
        auto v3 = make_var( ctx, 3 );
        auto v4 = make_var( ctx, 4 );

        auto s1 = lift( with( v1, v2 ), []( int a, int b ) { return a + b; } );

        auto s2 = lift( with( v3, v4 ), []( int a, int b ) { return a + b; } );

        auto s3 = with( s1, s2 ) | summ;

        CHECK_EQ( s1.get(), 3 );
        CHECK_EQ( s2.get(), 7 );
        CHECK_EQ( s3.get(), 10 );

        v1 <<= 10;
        v2 <<= 20;
        v3 <<= 30;
        v4 <<= 40;

        CHECK_EQ( s1.get(), 30 );
        CHECK_EQ( s2.get(), 70 );
        CHECK_EQ( s3.get(), 100 );

        bool b = false;

        b = is_signal<decltype( v1 )>::value;
        CHECK( b );

        b = is_signal<decltype( s1 )>::value;
        CHECK( b );

        b = is_signal<decltype( s2 )>::value;
        CHECK( b );

        b = is_signal<decltype( 10 )>::value;
        CHECK_FALSE( b );
    }

    TEST_CASE( "Signals2" )
    {
        context ctx;

        auto a1 = make_var( ctx, 1 );
        auto a2 = make_var( ctx, 1 );

        auto plus0 = ureact::lift( []( int value ) { return value + 0; } );
        auto summ = ureact::lift( []( int a, int b ) { return a + b; } );

        auto b1 = a1 | plus0;
        auto b2 = with( a1, a2 ) | summ;
        auto b3 = a2 | plus0;

        auto c1 = with( b1, b2 ) | summ;
        auto c2 = with( b2, b3 ) | summ;

        auto result = with( c1, c2 ) | summ;

        int observeCount = 0;

        observe( result, [&observeCount]( int v ) {
            observeCount++;
            if( observeCount == 1 )
                CHECK_EQ( v, 9 );
            else
                CHECK_EQ( v, 12 );
        } );

        CHECK_EQ( a1(), 1 );
        CHECK_EQ( a2(), 1 );

        CHECK_EQ( b1(), 1 );
        CHECK_EQ( b2(), 2 );
        CHECK_EQ( b3(), 1 );

        CHECK_EQ( c1(), 3 );
        CHECK_EQ( c2(), 3 );

        CHECK_EQ( result(), 6 );

        a1 <<= 2;

        CHECK_EQ( observeCount, 1 );

        CHECK_EQ( a1(), 2 );
        CHECK_EQ( a2(), 1 );

        CHECK_EQ( b1(), 2 );
        CHECK_EQ( b2(), 3 );
        CHECK_EQ( b3(), 1 );

        CHECK_EQ( c1(), 5 );
        CHECK_EQ( c2(), 4 );

        CHECK_EQ( result(), 9 );

        a2 <<= 2;

        CHECK_EQ( observeCount, 2 );

        CHECK_EQ( a1(), 2 );
        CHECK_EQ( a2(), 2 );

        CHECK_EQ( b1(), 2 );
        CHECK_EQ( b2(), 4 );
        CHECK_EQ( b3(), 2 );

        CHECK_EQ( c1(), 6 );
        CHECK_EQ( c2(), 6 );

        CHECK_EQ( result(), 12 );
    }

    TEST_CASE( "Signals3" )
    {
        context ctx;

        auto a1 = make_var( ctx, 1 );
        auto a2 = make_var( ctx, 1 );

        auto plus0 = ureact::lift( []( int value ) { return value + 0; } );
        auto summ = ureact::lift( []( int a, int b ) { return a + b; } );

        auto b1 = a1 | plus0;
        auto b2 = with( a1, a2 ) | summ;
        auto b3 = a2 | plus0;

        auto c1 = with( b1, b2 ) | summ;
        auto c2 = with( b2, b3 ) | summ;

        auto result = with( c1, c2 ) | summ;

        int observeCount = 0;

        observe( result, [&observeCount]( int v ) {
            observeCount++;
            CHECK_EQ( v, 12 );
        } );

        CHECK_EQ( a1(), 1 );
        CHECK_EQ( a2(), 1 );

        CHECK_EQ( b1(), 1 );
        CHECK_EQ( b2(), 2 );
        CHECK_EQ( b3(), 1 );

        CHECK_EQ( c1(), 3 );
        CHECK_EQ( c2(), 3 );

        CHECK_EQ( result(), 6 );

        do_transaction( ctx, [&] {
            a1 <<= 2;
            a2 <<= 2;
        } );

        CHECK_EQ( observeCount, 1 );

        CHECK_EQ( a1(), 2 );
        CHECK_EQ( a2(), 2 );

        CHECK_EQ( b1(), 2 );
        CHECK_EQ( b2(), 4 );
        CHECK_EQ( b3(), 2 );

        CHECK_EQ( c1(), 6 );
        CHECK_EQ( c2(), 6 );

        CHECK_EQ( result(), 12 );
    }

    TEST_CASE( "Signals4" )
    {
        context ctx;

        auto a1 = make_var( ctx, 1 );
        auto a2 = make_var( ctx, 1 );

        auto summ = ureact::lift( []( int a, int b ) { return a + b; } );

        auto b1 = with( a1, a2 ) | summ;
        auto b2 = with( b1, a2 ) | summ;

        CHECK_EQ( a1(), 1 );
        CHECK_EQ( a2(), 1 );

        CHECK_EQ( b1(), 2 );
        CHECK_EQ( b2(), 3 );

        a1 <<= 10;

        CHECK_EQ( a1(), 10 );
        CHECK_EQ( a2(), 1 );

        CHECK_EQ( b1(), 11 );
        CHECK_EQ( b2(), 12 );
    }

    TEST_CASE( "FunctionBind1" )
    {
        context ctx;

        auto v1 = make_var( ctx, 2 );
        auto v2 = make_var( ctx, 30 );
        auto v3 = make_var( ctx, 10 );

        auto signal = with( v1, v2, v3 )
                    | ureact::lift( [=]( int a, int b, int c ) -> int { return a * b * c; } );

        CHECK_EQ( signal(), 600 );
        v3 <<= 100;
        CHECK_EQ( signal(), 6000 );
    }

    TEST_CASE( "FunctionBind2" )
    {
        context ctx;

        auto a = make_var( ctx, 1 );
        auto b = make_var( ctx, 1 );

        auto summ = ureact::lift( []( int a, int b ) { return a + b; } );

        auto c = with( with( a, b ) | summ, with( a, make_var( ctx, 100 ) ) | summ )
               | ureact::lift( &myfunc );
        auto d = c | ureact::lift( &myfunc2 );
        auto e = with( d, d ) | ureact::lift( &myfunc3 );
        auto f = lift( e, []( float value ) { return -value + 100; } );

        CHECK_EQ( c(), 103 );
        CHECK_EQ( d(), 51.5f );
        CHECK_EQ( e(), 2652.25f );
        CHECK_EQ( f(), -2552.25 );

        a <<= 10;

        CHECK_EQ( c(), 121 );
        CHECK_EQ( d(), 60.5f );
        CHECK_EQ( e(), 3660.25f );
        CHECK_EQ( f(), -3560.25f );
    }

    TEST_CASE( "Flatten1" )
    {
        context ctx;

        auto inner1 = make_var( ctx, 123 );
        auto inner2 = make_var( ctx, 789 );

        auto outer = make_var( ctx, inner1 );

        auto flattened = flatten( outer );

        std::queue<int> results;

        observe( flattened, [&]( int v ) { results.push( v ); } );

        CHECK( outer().equals( inner1 ) );
        CHECK_EQ( flattened(), 123 );

        inner1 <<= 456;

        CHECK_EQ( flattened(), 456 );

        CHECK_EQ( results.front(), 456 );
        results.pop();
        CHECK( results.empty() );

        outer <<= inner2;

        CHECK( outer().equals( inner2 ) );
        CHECK_EQ( flattened(), 789 );

        CHECK_EQ( results.front(), 789 );
        results.pop();
        CHECK( results.empty() );
    }

    TEST_CASE( "Flatten2" )
    {
        context ctx;

        auto a0 = make_var( ctx, 100 );

        auto inner1 = make_var( ctx, 200 );

        auto plus0 = []( int value ) { return value + 0; };

        auto a1 = make_var( ctx, 300 );
        auto a2 = lift( a1, plus0 );
        auto a3 = lift( a2, plus0 );
        auto a4 = lift( a3, plus0 );
        auto a5 = lift( a4, plus0 );
        auto a6 = lift( a5, plus0 );
        auto inner2 = lift( a6, plus0 );

        CHECK_EQ( inner1(), 200 );
        CHECK_EQ( inner2(), 300 );

        auto outer = make_var( ctx, inner1 );

        auto flattened = flatten( outer );

        CHECK_EQ( flattened(), 200 );

        int observeCount = 0;

        observe( flattened, [&observeCount]( int v ) { observeCount++; } );

        auto o1 = lift( with( a0, flattened ), []( int a, int b ) { return a + b; } );
        auto o2 = lift( o1, plus0 );
        auto o3 = lift( o2, plus0 );
        auto result = lift( o3, plus0 );

        CHECK_EQ( result(), 100 + 200 );

        inner1 <<= 1234;

        CHECK_EQ( result(), 100 + 1234 );
        CHECK_EQ( observeCount, 1 );

        outer <<= inner2;

        CHECK_EQ( result(), 100 + 300 );
        CHECK_EQ( observeCount, 2 );

        do_transaction( ctx, [&] {
            a0 <<= 5000;
            a1 <<= 6000;
        } );

        CHECK_EQ( result(), 5000 + 6000 );
        CHECK_EQ( observeCount, 3 );
    }

    TEST_CASE( "Flatten3" )
    {
        auto plus0 = ureact::lift( []( int value ) { return value + 0; } );
        auto summ = ureact::lift( []( int a, int b ) { return a + b; } );

        context ctx;

        auto inner1 = make_var( ctx, 10 );

        auto a1 = make_var( ctx, 20 );
        auto a2 = a1 | plus0;
        auto a3 = a2 | plus0;
        auto inner2 = a3 | plus0;

        auto outer = make_var( ctx, inner1 );

        auto a0 = make_var( ctx, 30 );

        auto flattened = flatten( outer );

        int observeCount = 0;

        observe( flattened, [&observeCount]( int v ) { observeCount++; } );

        auto result = with( flattened, a0 ) | summ;

        CHECK_EQ( result(), 10 + 30 );
        CHECK_EQ( observeCount, 0 );

        do_transaction( ctx, [&] {
            inner1 <<= 1000;
            a0 <<= 200000;
            a1 <<= 50000;
            outer <<= inner2;
        } );

        CHECK_EQ( result(), 50000 + 200000 );
        CHECK_EQ( observeCount, 1 );

        do_transaction( ctx, [&] {
            a0 <<= 667;
            a1 <<= 776;
        } );

        CHECK_EQ( result(), 776 + 667 );
        CHECK_EQ( observeCount, 2 );

        do_transaction( ctx, [&] {
            inner1 <<= 999;
            a0 <<= 888;
        } );

        CHECK_EQ( result(), 776 + 888 );
        CHECK_EQ( observeCount, 2 );
    }

    TEST_CASE( "Flatten4" )
    {
        std::vector<int> results;

        auto plus0 = ureact::lift( []( int value ) { return value + 0; } );
        auto summ = ureact::lift( []( int a, int b ) { return a + b; } );

        context ctx;

        auto a1 = make_var( ctx, 100 );
        auto inner1 = a1 | plus0;

        auto a2 = make_var( ctx, 200 );
        auto inner2 = a2;

        auto a3 = make_var( ctx, 200 );

        auto outer = make_var( ctx, inner1 );

        auto flattened = flatten( outer );

        auto result = with( flattened, a3 ) | summ;

        observe( result, [&]( int v ) { results.push_back( v ); } );

        do_transaction( ctx, [&] {
            a3 <<= 400;
            outer <<= inner2;
        } );

        CHECK_EQ( results.size(), 1 );

        CHECK( std::find( results.begin(), results.end(), 600 ) != results.end() );
    }

    TEST_CASE( "Member1" )
    {
        context ctx;

        auto outer = make_var( ctx, 10 );
        auto inner = make_var( ctx, outer );

        auto flattened = flatten( inner );

        observe( flattened, []( int v ) { CHECK_EQ( v, 30 ); } );

        outer <<= 30;
    }

    TEST_CASE( "Modify1" )
    {
        context ctx;

        auto v = make_var( ctx, std::vector<int>{} );

        int obsCount = 0;

        observe( v, [&]( const std::vector<int>& v ) {
            CHECK_EQ( v, std::vector<int>{ 30, 50, 70 } );
            obsCount++;
        } );

        auto modificator = []( std::vector<int>& v ) {
            v.push_back( 30 );
            v.push_back( 50 );
            v.push_back( 70 );
        };

        SUBCASE( "method" )
        {
            v.modify( modificator );
        }
        SUBCASE( "operator" )
        {
            v <<= modificator;
        }

        CHECK_EQ( obsCount, 1 );
    }

    TEST_CASE( "Modify2" )
    {
        context ctx;

        auto v = make_var( ctx, std::vector<int>{} );

        int obsCount = 0;

        observe( v, [&]( const std::vector<int>& v ) {
            CHECK_EQ( v, std::vector<int>{ 30, 50, 70 } );
            obsCount++;
        } );

        do_transaction( ctx, [&] {
            v.modify( []( std::vector<int>& v ) { v.push_back( 30 ); } );

            v.modify( []( std::vector<int>& v ) { v.push_back( 50 ); } );

            v.modify( []( std::vector<int>& v ) { v.push_back( 70 ); } );
        } );


        CHECK_EQ( obsCount, 1 );
    }

    TEST_CASE( "Modify3" )
    {
        context ctx;

        auto vect = make_var( ctx, std::vector<int>{} );

        int obsCount = 0;

        observe( vect, [&]( const std::vector<int>& v ) {
            CHECK_EQ( v, std::vector<int>{ 30, 50, 70 } );
            obsCount++;
        } );

        // Also terrible
        do_transaction( ctx, [&] {
            vect.set( std::vector<int>{ 30, 50 } );

            vect.modify( []( std::vector<int>& v ) { v.push_back( 70 ); } );
        } );

        CHECK_EQ( obsCount, 1 );
    }

    TEST_CASE( "Signals of references" )
    {
        context ctx;

        Company company1( ctx, 1, "MetroTec" );
        Company company2( ctx, 2, "ACME" );

        Employee Bob( ctx, company1 );

        CHECK( Bob.company.get().get() == Company( ctx, 1, "MetroTec" ) );

        std::vector<std::string> bob_company_names;

        observe( Bob.company, [&]( const Company& company ) {
            bob_company_names.emplace_back( company.name.get() );
        } );

        // Changing ref of Bob's company to company2
        Bob.company <<= company2;

        CHECK( Bob.company.get().get() == Company( ctx, 2, "ACME" ) );

        CHECK( bob_company_names == std::vector<std::string>{ "ACME" } );
    }

    TEST_CASE( "Signal of events" )
    {
        context ctx;

        auto in1 = make_source<int>( ctx );
        auto in2 = make_source<int>( ctx );

        auto sig = make_var( ctx, in1 );

        int reassign_count = 0;

        observe( sig, [&]( const events<int>& ) { ++reassign_count; } );

        auto f = flatten( sig );

        std::vector<int> saved_events;

        observe( f, [&]( const int value ) { saved_events.push_back( value ); } );

        in1 << -1;
        in2 << 1;

        sig <<= in2;

        in1 << -2;
        in2 << 2;

        CHECK( saved_events == std::vector<int>{ -1, 2 } );
        CHECK( reassign_count == 1 );
    }
}

namespace
{
// Helpers
using ExprPairT = std::pair<std::string, int>;
using ExprVectT = std::vector<ExprPairT>;

std::string makeExprStr( int a, int b, const char* op )
{
    return std::to_string( a ) + std::string( op ) + std::to_string( b );
}

ExprPairT makeExprPair( const std::string& s, int v )
{
    return make_pair( s, v );
}

std::string makeExpressionsString( const ExprVectT& expressions )
{
    std::ostringstream ss;
    ss << "Expressions:\n";
    for( const auto& p : expressions )
    {
        ss << "* " << p.first << " is " << p.second << "\n";
    }
    return ss.str();
}

ureact::signal<ExprVectT> createExpressionSignal(
    const ureact::signal<int>& a, const ureact::signal<int>& b )
{
    // Inside a function, we can use auto
    const auto sumExpr = lift(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "+" ); } );

    const auto diffExpr = lift(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "-" ); } );

    const auto prodExpr = lift(
        with( a, b ), []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "*" ); } );

    return lift(
        // clang-format off
            with(
                lift( with( sumExpr,  a + b  ), &makeExprPair ),
                lift( with( diffExpr, a - b ),  &makeExprPair ),
                lift( with( prodExpr, a * b ),  &makeExprPair )
            ),
        // clang-format on
        []( const ExprPairT& sumP, const ExprPairT& diffP, const ExprPairT& prodP ) {
            return ExprVectT{ sumP, diffP, prodP };
        } );
}
} // namespace

TEST_SUITE( "Examples" )
{
    TEST_CASE( "Complex signals" )
    {
        ureact::context ctx;

        // Input operands
        ureact::var_signal<int> a = make_var( ctx, 1 );
        ureact::var_signal<int> b = make_var( ctx, 2 );

        // The expression std::vector
        ureact::signal<ExprVectT> expressions;

        // Several alternative variants that do exactly the same
        SUBCASE( "intermediate signals" )
        {
            // Calculations
            ureact::signal<int> sum = a + b;
            ureact::signal<int> diff = a - b;
            ureact::signal<int> prod = a * b;

            // stringified expressions
            ureact::signal<std::string> sumExpr = lift( with( a, b ),
                []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "+" ); } );

            ureact::signal<std::string> diffExpr = lift( with( a, b ),
                []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "-" ); } );

            ureact::signal<std::string> prodExpr = lift( with( a, b ),
                []( const int lhs, const int rhs ) { return makeExprStr( lhs, rhs, "*" ); } );

            expressions = lift(
                // clang-format off
                with(
                    lift( with( sumExpr,  sum  ), &makeExprPair ),
                    lift( with( diffExpr, diff ), &makeExprPair ),
                    lift( with( prodExpr, prod ), &makeExprPair )
                ),
                // clang-format on
                []( const ExprPairT& sumP, const ExprPairT& diffP, const ExprPairT& prodP ) {
                    return ExprVectT{ sumP, diffP, prodP };
                } );
        }

        SUBCASE( "intermediate signals in a function" )
        {
            expressions = createExpressionSignal( a, b );
        }

        SUBCASE( "imperative function" )
        {
            expressions = lift( with( a, b ), []( int a_, int b_ ) {
                return ExprVectT{
                    make_pair( makeExprStr( a_, b_, "+" ), a_ + b_ ),
                    make_pair( makeExprStr( a_, b_, "-" ), a_ - b_ ),
                    make_pair( makeExprStr( a_, b_, "*" ), a_ * b_ ),
                };
            } );
        }

        REQUIRE( expressions.is_valid() );

        ureact::signal<std::string> expressionsString = lift( expressions, &makeExpressionsString );

        CHECK( expressionsString.get() ==
               R"(Expressions:
* 1+2 is 3
* 1-2 is -1
* 1*2 is 2
)" );

        a <<= 10;

        CHECK( expressionsString.get() ==
               R"(Expressions:
* 10+2 is 12
* 10-2 is 8
* 10*2 is 20
)" );

        b <<= 20;

        CHECK( expressionsString.get() ==
               R"(Expressions:
* 10+20 is 30
* 10-20 is -10
* 10*20 is 200
)" );
    }
}

TEST_SUITE( "Examples" )
{
    TEST_CASE( "Reactive class members" )
    {
        ureact::context ctx;

        class Shape
        {
        public:
            explicit Shape( ureact::context& ctx )
                : width( make_var( ctx, 0 ) )
                , height( make_var( ctx, 0 ) )
                , size( width * height )
            {}

            ureact::var_signal<int> width;
            ureact::var_signal<int> height;

            ureact::signal<int> size;
        };

        Shape my_shape( ctx );

        CHECK( my_shape.width.get() == 0 );
        CHECK( my_shape.height.get() == 0 );
        CHECK( my_shape.size.get() == 0 );

        std::vector<int> size_values;

        auto on_size_value_change
            = [&]( const int new_value ) { size_values.push_back( new_value ); };

        observe( my_shape.size, on_size_value_change );

        CHECK( size_values == std::vector<int>{} );

        // Do transaction to change width and height in single step
        do_transaction( ctx, [&] {
            my_shape.width <<= 4;
            my_shape.height <<= 4;
        } );

        CHECK( size_values == std::vector<int>{ 16 } );

        CHECK( my_shape.width.get() == 4 );
        CHECK( my_shape.height.get() == 4 );
        CHECK( my_shape.size.get() == 16 );
    }

    TEST_CASE( "Signals of references" )
    {
        ureact::context ctx;

        Company company1( ctx, 1, "MetroTec" );
        Company company2( ctx, 2, "ACME" );

        Employee Bob( ctx, company1 );

        CHECK( Bob.company.get().get() == Company( ctx, 1, "MetroTec" ) );

        std::vector<std::string> bob_company_names;

        observe( Bob.company, [&]( const Company& company ) {
            bob_company_names.emplace_back( company.name.get() );
        } );

        // Changing ref of Bob's company to company2
        Bob.company <<= company2;

        CHECK( Bob.company.get().get() == Company( ctx, 2, "ACME" ) );

        CHECK( bob_company_names == std::vector<std::string>{ "ACME" } );
    }

    TEST_CASE( "Dynamic signal references" )
    {
        ureact::context ctx;

        Company company1( ctx, 1, "MetroTec" );
        Company company2( ctx, 2, "ACME" );

        Employee Alice( ctx, company1 );

        CHECK( Alice.company.get() == Company( ctx, 1, "MetroTec" ) );

        std::vector<std::string> alice_company_names;

        // Observer isn't bound to long term lived signal. So we need to keep it alive
        auto obs = observe( ureact::reactive_ref( Alice.company, &Company::name ),
            [&]( const std::string& name ) { alice_company_names.emplace_back( name ); } );

        company1.name <<= std::string( "ModernTec" );
        Alice.company <<= std::ref( company2 );
        company2.name <<= std::string( "A.C.M.E." );

        CHECK( alice_company_names == std::vector<std::string>{ "ModernTec", "ACME", "A.C.M.E." } );
    }
}

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
                auto my_signal = lift( x, identity );

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
                    auto my_signal = lift( x, identity );

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

TEST_SUITE( "Examples" )
{
    TEST_CASE( "Hello World" )
    {
        ureact::context ctx;

        // make_var is available as a free function and as context's member function
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

        SUBCASE( "using lift()" )
        {
            bothWords = lift( with( firstWord, secondWord ), concatFunc );
        }

        SUBCASE( "operators , and |" )
        {
            // operator "|" can be used instead of lift()
            bothWords = with( firstWord, secondWord ) | ureact::lift( concatFunc );
        }

        // Imperative value access
        CHECK( bothWords.get() == "Change me!" );

        // Imperative value change
        firstWord <<= std::string( "Hello" );

        CHECK( bothWords.get() == "Hello me!" );

        secondWord <<= std::string( "World!" );

        CHECK( bothWords.get() == "Hello World!" );
    }

    TEST_CASE( "Modifying signal values in place" )
    {
        ureact::context ctx;

        ureact::var_signal<std::vector<std::string>> data
            = make_var( ctx, std::vector<std::string>{} );

        CHECK( data.get() == std::vector<std::string>{} );

        data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "Hello" ); } );

        CHECK( data.get() == std::vector<std::string>{ "Hello" } );

        data.modify( []( std::vector<std::string>& value ) { value.emplace_back( "World!" ); } );

        CHECK( data.get() == std::vector<std::string>{ "Hello", "World!" } );
    }

    TEST_CASE( "Changing multiple inputs" )
    {
        ureact::context ctx;

        ureact::var_signal<int> a = make_var( ctx, 1 );
        ureact::var_signal<int> b = make_var( ctx, 1 );

        ureact::signal<int> x = a + b;
        ureact::signal<int> y = a + b;
        ureact::signal<int> z = x + y; // 2*(a+b)

        //  a b  //
        //  /X\  //
        // x   y //
        //  \ /  //
        //   z   //

        std::vector<int> z_values;
        observe( z, [&]( int new_value ) { z_values.push_back( new_value ); } );

        CHECK( z.get() == 4 );
        CHECK( z_values == std::vector<int>{} );

        a <<= 2; // z is changed to 6
        b <<= 2; // z is changed to 8

        do_transaction( ctx, [&] {
            a <<= 4;
            b <<= 4;
        } ); // z is changed to 16

        CHECK( z_values == std::vector<int>{ 6, 8, 16 } );
    }

    TEST_CASE( "Reacting to value changes" )
    {
        ureact::context ctx;

        ureact::var_signal<int> x = make_var( ctx, 1 );
        ureact::signal<int> xAbs = lift( x, []( int value ) { return abs( value ); } );

        std::vector<int> xAbs_values;
        observe( xAbs, [&]( int new_value ) { xAbs_values.push_back( new_value ); } );

        CHECK( xAbs.get() == 1 );
        CHECK( xAbs_values == std::vector<int>{} );

        x <<= 2;  // xAbs is changed to 2
        x <<= -3; // xAbs is changed to 3
        x <<= 3;  // no output, xAbs is still 3

        do_transaction( ctx, [&] {
            x <<= 4;
            x <<= -2;
            x <<= 3;
        } ); // no output, result value of xAbs is still 3

        CHECK( xAbs_values == std::vector<int>{ 2, 3 } );
    }
}
