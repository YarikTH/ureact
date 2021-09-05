//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

namespace
{
class Company
{
public:
    int index;
    ureact::value<std::string> name;

    Company( ureact::context& ctx, const int index, const char* name )
        : index( index )
        , name( make_value( ctx, std::string( name ) ) )
    {}

    friend bool operator==( const Company& lhs, const Company& rhs )
    {
        // clang-format off
            return std::tie( lhs.index, lhs.name.get() )
                == std::tie( rhs.index, rhs.name.get() );
        // clang-format on
    }
};

std::ostream& operator<<( std::ostream& os, const Company& company )
{
    os << "Company{ index: " << company.index << ", name: \"" << company.name.get() << "\" }";
    return os;
}

class Employee
{
public:
    ureact::value<Company&> company;

    Employee( ureact::context& ctx, Company& company )
        : company( make_value( ctx, std::ref( company ) ) )
    {}
};

} // namespace

TEST_SUITE( "Examples" )
{
    TEST_CASE( "Reactive class members" )
    {
        ureact::context ctx;

        class Shape
        {
        public:
            explicit Shape( ureact::context& ctx )
                : width( make_value( ctx, 0 ) )
                , height( make_value( ctx, 0 ) )
                , size( width * height )
            {}

            ureact::value<int> width;
            ureact::value<int> height;

            ureact::function<int> size;
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
        ctx.do_transaction( [&] {
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
