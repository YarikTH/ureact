#include <iostream>

#include "ureact/ureact.hpp"

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

std::ostream& operator<<( std::ostream& os, const Shape& shape )
{
    // clang-format off
    os << "Shape{ width: "  << shape.width.value()
            << ", height: " << shape.height.value()
            << ", size: "   << shape.size.value()
            << " }";
    // clang-format on
    return os;
}

int main()
{
    std::cout << "======================\n";
    std::cout << "Reactive class members\n";
    std::cout << "======================\n";
    std::cout << "\n";

    ureact::context ctx;

    Shape myShape( ctx );

    std::cout << "-------------\n";
    std::cout << "Initial state\n";
    std::cout << "-------------\n";
    std::cout << "myShape: " << myShape << "\n";
    std::cout << "\n";

    observe( myShape.size,
        []( int new_value ) { std::cout << ">> size changed to " << new_value << "\n"; } );

    std::cout << "--------------------------------------------------------\n";
    std::cout << "Do transaction to change width and height in single step\n";
    std::cout << "--------------------------------------------------------\n";

    ctx.do_transaction( [&] {
        myShape.width <<= 4;
        myShape.height <<= 4;
    } );

    std::cout << "myShape: " << myShape << "\n";
}
