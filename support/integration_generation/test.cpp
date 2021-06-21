#include <ureact/ureact.hpp>

int main()
{
    ureact::context ctx;

    ureact::var_signal<int> b = ctx.make_var( 1 );
    ureact::var_signal<int> c = ctx.make_var( 2 );
    ureact::signal<int> a = b + c;
    b <<= 10;

    return ( a.value() == 12 ) ? 0 : 1;
}
