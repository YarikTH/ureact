//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// non-copyable and non-movable
static_assert( std::is_default_constructible_v<ureact::context> );
static_assert( !std::is_copy_constructible_v<ureact::context> );
static_assert( !std::is_copy_assignable_v<ureact::context> );
static_assert( !std::is_move_constructible_v<ureact::context> );
static_assert( !std::is_move_assignable_v<ureact::context> );
