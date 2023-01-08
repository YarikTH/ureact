//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <utility>

// Based on std::identity from c++20
// https://en.cppreference.com/w/cpp/utility/functional/identity
struct identity
{
    template <class T>
    constexpr auto&& operator()( T&& t ) const noexcept
    {
        return std::forward<T>( t );
    }
};
