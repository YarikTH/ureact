#include <iostream>
#include <string>
#include <utility>

#include "ureact/signal.hpp"
#include "ureact/observer.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Example 1 - Reactive class members
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace example1
{
    class Shape
    {
    public:
        explicit Shape( ureact::context* c )
            : Width( make_var(c, 0) )
            , Height( make_var(c, 0) )
            , Size( Width * Height )
        {
        }
        
        ureact::var_signal<int>     Width;
        ureact::var_signal<int>     Height;

        ureact::signal<int>        Size;
    };

    void Run()
    {
        std::cout << "Example 1 - Reactive class members" << std::endl;
        
        ureact::context c;
        
        Shape myShape( &c );

        observe(myShape.Size, [] (int new_value) {
            std::cout << "Size changed to " << new_value << std::endl;
        });

        c.do_transaction([&] {
            myShape.Width  <<= 4;
            myShape.Height <<= 4; 
        }); // output: Size changed to 16

        std::cout << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Example 2 - Signals of references
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace example2
{
    class Company
    {
    public:
        const char* Name;

        Company(const char* name) :
            Name( name )
        {}

        // Note: To be used as a signal value type,
        // values of the type must be comparable
        bool operator==(const Company& other) const
        {
            return this == &other;
        }
    };

    class Employee
    {
    public:
        ureact::var_signal<Company&> MyCompany;

        Employee(ureact::context* c, Company& company) :
            MyCompany( make_var(c, std::ref(company)) )
        {}
    };

    void Run()
    {
        std::cout << "Example 2 - Signals of references" << std::endl;

        ureact::context c;
        
        Company     company1( "MetroTec" );
        Company     company2( "ACME" );

        Employee    bob( &c, company1 );

        observe(bob.MyCompany, [] (const Company& company) {
            std::cout << "Bob works for " << company.Name << std::endl;
        });

        bob.MyCompany <<= std::ref(company2); // output: Bob now works for ACME

        std::cout << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Example 3 - Dynamic signal references
///////////////////////////////////////////////////////////////////////////////////////////////////
namespace example3
{
    class Company
    {
    public:
        ureact::var_signal<std::string> Name;

        Company(ureact::context* c, const char* name) :
            Name( make_var(c, std::string( name )) )
        {}

        bool operator==(const Company& other) const
        {
            return this == &other;
        }
    };

    class Employee
    {
    public:
        ureact::var_signal<Company&> MyCompany;

        Employee(ureact::context* c, Company& company) :
            MyCompany( make_var(c, std::ref(company)) )
        {}
    };

    void Run()
    {
        std::cout << "Example 3 - Dynamic signal references" << std::endl;
        
        ureact::context c;
        
        Company     company1( &c, "MetroTec" );
        Company     company2( &c, "ACME" );

        Employee    alice( &c, company1 );

        auto obs = observe(
            REACTIVE_REF(alice.MyCompany, Name),
            [] (const std::string& name) {
                std::cout << "Alice now works for " << name << std::endl;
            });

        company1.Name   <<= std::string( "ModernTec" );  // output: Alice now works for ModernTec
        alice.MyCompany <<= std::ref(company2);          // output: Alice now works for ACME
        company2.Name   <<= std::string( "A.C.M.E." );   // output: Alice now works for A.C.M.E.

        std::cout << std::endl;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Run examples
///////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
    example1::Run();

    example2::Run();

    example3::Run();

    return 0;
}