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
