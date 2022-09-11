//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/flatten.hpp"

#include <algorithm>
#include <queue>
#include <variant>

#include "doctest_extra.h"
#include "ureact/observe.hpp"
#include "ureact/transaction.hpp"

// TODO: replace FlattenN tests with more meaningful ones

TEST_CASE( "Flatten1" )
{
    ureact::context ctx;

    auto inner1 = make_var( ctx, 123 );
    auto inner2 = make_var( ctx, 789 );

    auto outer = make_var( ctx, inner1 );

    auto flattened = flatten( outer );

    std::queue<int> results;

    observe( flattened, [&]( int v ) { results.push( v ); } );

    CHECK( outer.get().equal_to( inner1 ) );
    CHECK( flattened.get() == 123 );

    inner1 <<= 456;

    CHECK( flattened.get() == 456 );

    CHECK( results.front() == 456 );
    results.pop();
    CHECK( results.empty() );

    outer <<= inner2;

    CHECK( outer.get().equal_to( inner2 ) );
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

    const bool valueFound = std::find( results.begin(), results.end(), 600 ) != results.end();
    CHECK( valueFound );
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

    CHECK( outer.get().equal_to( inner1 ) );
    CHECK( flattened.get() == 123 );

    CHECK( results.empty() );

    outer <<= inner2;

    CHECK( outer.get().equal_to( inner2 ) );
    CHECK( flattened.get() == 123 );

    // flattened observer shouldn't trigger if value isn't changed
    CHECK( results.empty() );
}

TEST_CASE( "Member1" )
{
    ureact::context ctx;

    auto outer = make_var( ctx, 10 );
    auto inner = make_var( ctx, outer );

    auto flattened = flatten( inner );

    observe( flattened, []( int v ) { CHECK_EQ( v, 30 ); } );

    outer <<= 30;
}

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

private:
    friend std::ostream& operator<<( std::ostream& os, const Company& company )
    {
        os << "Company{ index: " << company.index << ", name: \"" << company.name.get() << "\" }";
        return os;
    }
};

class Employee
{
public:
    ureact::var_signal<std::reference_wrapper<Company>> company;

    Employee( ureact::context& ctx, Company& company_ref )
        : company( make_var( ctx, std::ref( company_ref ) ) )
    {}

private:
    friend std::ostream& operator<<( std::ostream& os, const Employee& employee )
    {
        os << "Employee{ company: " << employee.company.get() << " }";
        return os;
    }
};

class Employee2
{
public:
    ureact::var_signal<Company*> company;

    Employee2( ureact::context& ctx, Company* company_ptr )
        : company( make_var( ctx, company_ptr ) )
    {}

private:
    friend std::ostream& operator<<( std::ostream& os, const Employee2& employee )
    {
        os << "Employee{ company: " << *employee.company.get() << " }";
        return os;
    }
};

} // namespace

TEST_CASE( "DynamicSignalRefOrPtr" )
{
    ureact::context ctx;

    Company company1( ctx, 1, "MetroTec" );
    Company company2( ctx, 2, "ACME" );

    std::variant<std::monostate, Employee, Employee2> Alice;

    ureact::signal<std::string> alice_company_name;

    SUBCASE( "Reference" )
    {
        Alice = Employee{ ctx, company1 };

        alice_company_name
            = ureact::reactive_ref( std::get<Employee>( Alice ).company, &Company::name );
    }
    SUBCASE( "Pointer" )
    {
        Alice = Employee2{ ctx, &company1 };

        alice_company_name
            = ureact::reactive_ref( std::get<Employee2>( Alice ).company, &Company::name );
    }

    std::vector<std::string> alice_company_names;

    observe( alice_company_name,
        [&]( const std::string& name ) { alice_company_names.push_back( name ); } );

    // assign company reference or pointer depends on type of employee
    auto make_company_change_visitor = []( Company& company ) {
        return [&company]( auto&& employee ) {
            using T = std::decay_t<decltype( employee )>;
            if constexpr( std::is_same_v<T, Employee> )
                employee.company <<= std::ref( company );
            else if constexpr( std::is_same_v<T, Employee2> )
                employee.company <<= &company;
        };
    };

    company1.name <<= std::string( "ModernTec" );
    std::visit( make_company_change_visitor( company2 ), Alice );
    company2.name <<= std::string( "A.C.M.E." );

    CHECK( alice_company_names == std::vector<std::string>{ "ModernTec", "ACME", "A.C.M.E." } );
}

TEST_CASE( "Signal of events" )
{
    ureact::context ctx;

    auto in1 = ureact::make_source<int>( ctx );
    auto in2 = ureact::make_source<int>( ctx );

    auto sig = make_var( ctx, in1 );

    int reassign_count = 0;

    observe( sig, [&]( const ureact::events<int>& ) { ++reassign_count; } );

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
