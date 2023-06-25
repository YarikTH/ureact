//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/reactive_ref.hpp"

#include <iostream>
#include <tuple>

#include "catch2_extra.hpp"
#include "ureact/adaptor/observe.hpp"

namespace
{

// TODO: think about moving it into a separate header shared between several test cases
class Company : ureact::member_signal_user<Company>
{
public:
    int index;
    member_var_signal<std::string> name;

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

class Employee : ureact::member_signal_user<Employee>
{
public:
    member_var_signal<std::reference_wrapper<Company>> company;

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

} // namespace

TEST_CASE( "ureact::reactive_ref" )
{
    ureact::context ctx;

    Company company1( ctx, 1, "MetroTec" );
    Company company2( ctx, 2, "ACME" );

    ureact::signal<std::string> alice_company_name;

    auto Alice = Employee{ ctx, company1 };
    auto alice_company = Alice.company;

    SECTION( "Functional syntax" )
    {
        alice_company_name = ureact::reactive_ref( alice_company, &Company::name );
    }
    SECTION( "Piped syntax" )
    {
        alice_company_name = alice_company | ureact::reactive_ref( &Company::name );
    }

    std::vector<std::string> alice_company_names;

    ureact::observe( alice_company_name,
        [&]( const std::string& name ) { alice_company_names.push_back( name ); } );

    company1.name <<= std::string( "ModernTec" );
    Alice.company <<= std::ref( company2 );
    company2.name <<= std::string( "A.C.M.E." );

    CHECK( alice_company_names == std::vector<std::string>{ "ModernTec", "ACME", "A.C.M.E." } );
}
