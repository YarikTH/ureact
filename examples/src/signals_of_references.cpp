#include <iostream>

#include "ureact/ureact.hpp"

class Company
{
public:
    int index;
    const char* name;
    
    explicit Company( const int aindex, const char* aname )
        : index( aindex )
        , name( aname )
    {
    }
    
    friend bool operator==( const Company& lhs, const Company& rhs )
    {
        return std::tie( lhs.index, lhs.name )
            == std::tie( rhs.index, rhs.name );
    }
};

std::ostream& operator<<( std::ostream& os, const Company& company )
{
    os << "Company{ index: " << company.index << ", name: \"" << company.name << "\" }";
    return os;
}


class Employee
{
public:
    ureact::var_signal<Company&> company;
    
    Employee( ureact::context* c, Company& acompany )
        : company( make_var( c, std::ref( acompany ) ) )
    {
    }
};

std::ostream& operator<<( std::ostream& os, const Employee& employee )
{
    os << "Employee{ company: "  << employee.company.value() << " }";
    return os;
}


int main()
{
    std::cout << "=====================\n";
    std::cout << "Signals of references\n";
    std::cout << "=====================\n";
    std::cout << "\n";
    
    ureact::context c;
    
    Company company1( 1, "MetroTec" );
    Company company2( 2, "ACME" );
    
    Employee Bob( &c, company1 );
    
    std::cout << "-------------\n";
    std::cout << "Initial state\n";
    std::cout << "-------------\n";
    std::cout << "company1: " << company1 << "\n";
    std::cout << "company2: " << company2 << "\n";
    std::cout << "Bob: " << Bob << "\n";
    std::cout << "\n";
    
    observe( Bob.company, []( const Company& company )
    {
        std::cout << ">> Bob works for " << company.name << "\n";
    } );
    
    std::cout << "-----------------------------------------\n";
    std::cout << "Changing ref of Bob's company to company2\n";
    std::cout << "-----------------------------------------\n";
    Bob.company <<= company2;
    
    std::cout << "Bob: " << Bob << "\n";
}
