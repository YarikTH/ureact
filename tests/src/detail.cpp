//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "doctest_extra.h"
#include "ureact/ureact.hpp"

// non-copyable and non-movable
static_assert( !std::is_copy_constructible_v<ureact::detail::node_base> );
static_assert( !std::is_copy_assignable_v<ureact::detail::node_base> );
static_assert( !std::is_move_constructible_v<ureact::detail::node_base> );
static_assert( !std::is_move_assignable_v<ureact::detail::node_base> );

// copyable and nothrow movable
static_assert( std::is_default_constructible_v<ureact::detail::reactive_base<int>> );
static_assert( std::is_copy_constructible_v<ureact::detail::reactive_base<int>> );
static_assert( std::is_copy_assignable_v<ureact::detail::reactive_base<int>> );
static_assert( std::is_move_constructible_v<ureact::detail::reactive_base<int>> );
static_assert( std::is_move_assignable_v<ureact::detail::reactive_base<int>> );
static_assert( std::is_nothrow_move_constructible_v<ureact::detail::reactive_base<int>> );
static_assert( std::is_nothrow_move_assignable_v<ureact::detail::reactive_base<int>> );
