#include "tests_stdafx.hpp"
#include "ureact/ureact.hpp"

namespace
{
class Company
{
public:
    int index;
    ureact::var_signal<std::string> name;

    Company( ureact::context& ctx, const int aindex, const char* aname )
        : index( aindex )
        , name( make_var( ctx, std::string( aname ) ) )
    {}

    friend bool operator==( const Company& lhs, const Company& rhs )
    {
        // clang-format off
            return std::tie( lhs.index, lhs.name.value() )
                == std::tie( rhs.index, rhs.name.value() );
        // clang-format on
    }
};

std::ostream& operator<<( std::ostream& os, const Company& company )
{
    os << "Company{ index: " << company.index << ", name: \"" << company.name.value() << "\" }";
    return os;
}

class Employee
{
public:
    ureact::var_signal<Company&> company;

    Employee( ureact::context& ctx, Company& acompany )
        : company( make_var( ctx, std::ref( acompany ) ) )
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
                : width( make_var( ctx, 0 ) )
                , height( make_var( ctx, 0 ) )
                , size( width * height )
            {}

            ureact::var_signal<int> width;
            ureact::var_signal<int> height;

            ureact::signal<int> size;
        };

        Shape my_shape( ctx );

        CHECK( my_shape.width.value() == 0 );
        CHECK( my_shape.height.value() == 0 );
        CHECK( my_shape.size.value() == 0 );

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

        CHECK( my_shape.width.value() == 4 );
        CHECK( my_shape.height.value() == 4 );
        CHECK( my_shape.size.value() == 16 );
    }

    TEST_CASE( "Signals of references" )
    {
        ureact::context ctx;

        Company company1( ctx, 1, "MetroTec" );
        Company company2( ctx, 2, "ACME" );

        Employee Bob( ctx, company1 );

        CHECK( Bob.company.value().get() == Company( ctx, 1, "MetroTec" ) );

        std::vector<std::string> bob_company_names;

        observe( Bob.company, [&]( const Company& company ) {
            bob_company_names.emplace_back( company.name.value() );
        } );

        // Changing ref of Bob's company to company2
        Bob.company <<= company2;

        CHECK( Bob.company.value().get() == Company( ctx, 2, "ACME" ) );

        CHECK( bob_company_names == std::vector<std::string>{ "ACME" } );
    }

    TEST_CASE( "Dynamic signal references" )
    {
        ureact::context ctx;

        Company company1( ctx, 1, "MetroTec" );
        Company company2( ctx, 2, "ACME" );

        Employee Alice( ctx, company1 );

        CHECK( Alice.company.value() == Company( ctx, 1, "MetroTec" ) );

        std::vector<std::string> alice_company_names;

        // Observer isn't bound to long term lived signal. So we need to keep it alive
        auto obs = observe( UREACT_REACTIVE_REF( Alice.company, name ),
            [&]( const std::string& name ) { alice_company_names.emplace_back( name ); } );

        company1.name <<= std::string( "ModernTec" );
        Alice.company <<= std::ref( company2 );
        company2.name <<= std::string( "A.C.M.E." );

        CHECK( alice_company_names == std::vector<std::string>{ "ModernTec", "ACME", "A.C.M.E." } );
    }
}
