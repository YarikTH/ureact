//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/default_context.hpp"

#include "doctest_extra.h"
#include "ureact/adaptor/collect.hpp"
#include "ureact/adaptor/merge.hpp"
#include "ureact/events.hpp"
#include "ureact/signal.hpp"

TEST_CASE( "DefaultContext" )
{
    using namespace ureact::default_context;

    {
        ureact::var_signal var = make_var( 1 );
        CHECK( var.get() == 1 );
        var <<= 2;
        CHECK( var.get() == 2 );
    }

    {
        auto cnst = make_const( 2 );
        CHECK( cnst.get() == 2 );
    }

    {
        auto src = make_source<int>();
        auto nev = make_never<int>();
        auto events = ureact::merge( src, nev );
        auto result = ureact::collect<std::vector>( events );

        src << 1 << 2 << 3;
        CHECK( result.get() == std::vector{ 1, 2, 3 } );
    }
}
