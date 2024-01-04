
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <ureact/adaptor/lift.hpp>
#include <ureact/signal.hpp>

int main()
{
    ureact::context ctx;

    ureact::var_signal<int> b = make_var( ctx, 1 );
    ureact::var_signal<int> c = make_var( ctx, 2 );
    ureact::signal<int> a = b + c;
    b <<= 10;

    return ( a.get() == 12 ) ? 0 : 1;
}
