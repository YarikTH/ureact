#include "doctest/doctest.h"
#include "ureact/ureact.hpp"
#include <string>

namespace
{

class Company
{
public:
    int index;
    ureact::var_signal<std::string> name;

    Company( ureact::context* c, const int aindex, const char* aname )
        : index( aindex )
        , name( make_var( c, std::string( aname ) ) )
    {}

    friend bool operator==( const Company& lhs, const Company& rhs )
    {
        // clang-format off
        return std::tie( lhs.index, lhs.name.value() )
            == std::tie( rhs.index, rhs.name.value() );
        // clang-format on
    }
};

class Employee
{
public:
    ureact::var_signal<Company&> company;

    Employee( ureact::context* ctx, Company& companyRef )
        : company( make_var( ctx, std::ref( companyRef ) ) )
    {}
};

class Employee2
{
public:
    ureact::var_signal<Company*> company;

    Employee2( ureact::context* ctx, Company* companyPtr )
        : company( make_var( ctx, companyPtr ) )
    {}
};

} // namespace

TEST_SUITE_BEGIN( "dynamic_signals_test" );

TEST_CASE( "DynamicSignalReferences" )
{
    ureact::context ctx;

    Company company1( &ctx, 1, "MetroTec" );
    Company company2( &ctx, 2, "ACME" );

    Employee Alice( &ctx, company1 );

    ureact::signal<std::string> aliceCompanyName = REACTIVE_REF( Alice.company, name );

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

    Company company1( &ctx, 1, "MetroTec" );
    Company company2( &ctx, 2, "ACME" );

    Employee2 Alice( &ctx, &company1 );

    ureact::signal<std::string> aliceCompanyName = REACTIVE_PTR( Alice.company, name );

    std::vector<std::string> result;

    observe( aliceCompanyName, [&]( const std::string& name ) { result.push_back( name ); } );

    company1.name <<= std::string( "ModernTec" );
    Alice.company <<= &company2;
    company2.name <<= std::string( "A.C.M.E." );

    CHECK( result == std::vector<std::string>{ "ModernTec", "ACME", "A.C.M.E." } );
}

TEST_SUITE_END();
