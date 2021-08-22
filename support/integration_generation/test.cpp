
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <ureact/ureact.hpp>

int main()
{
    ureact::context ctx;

    ureact::value<int> b = ctx.make_value( 1 );
    ureact::value<int> c = ctx.make_value( 2 );
    ureact::signal<int> a = b + c;
    b <<= 10;

    return ( a.get() == 12 ) ? 0 : 1;
}
