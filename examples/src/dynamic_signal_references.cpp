#include <iostream>
#include <string>

#include "ureact/ureact.hpp"

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

std::ostream& operator<<( std::ostream& os, const Employee& employee )
{
    os << "Employee{ company: " << employee.company.value() << " }";
    return os;
}

int main()
{
    std::cout << "=========================\n";
    std::cout << "Dynamic signal references\n";
    std::cout << "=========================\n";
    std::cout << "\n";

    ureact::context ctx;

    Company company1( ctx, 1, "MetroTec" );
    Company company2( ctx, 2, "ACME" );

    Employee Alice( ctx, company1 );

    std::cout << "-------------\n";
    std::cout << "Initial state\n";
    std::cout << "-------------\n";
    std::cout << "company1: " << company1 << "\n";
    std::cout << "company2: " << company2 << "\n";
    std::cout << "Alice: " << Alice << "\n";
    std::cout << "\n";

    auto obs = observe( REACTIVE_REF( Alice.company, name ), []( const std::string& name ) {
        std::cout << ">> Alice now works for \"" << name << "\"\n";
    } );

    std::cout << "----------------------------------------\n";
    std::cout << "Changing name of company1 to \"ModernTec\"\n";
    std::cout << "----------------------------------------\n";
    company1.name <<= std::string( "ModernTec" );
    std::cout << "Alice: " << Alice << "\n";
    std::cout << "\n";

    std::cout << "-------------------------------------------\n";
    std::cout << "Changing ref of Alice's company to company2\n";
    std::cout << "-------------------------------------------\n";
    Alice.company <<= std::ref( company2 );
    std::cout << "Alice: " << Alice << "\n";
    std::cout << "\n";

    std::cout << "---------------------------------------\n";
    std::cout << "Changing name of company2 to \"A.C.M.E.\"\n";
    std::cout << "---------------------------------------\n";
    company2.name <<= std::string( "A.C.M.E." );
    std::cout << "Alice: " << Alice << "\n";
}
