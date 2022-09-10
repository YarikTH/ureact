//
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <utility>

// Based on std::identity from c++20
// https://en.cppreference.com/w/cpp/utility/functional/identity
// The only difference is return type that is auto instead of auto&&
// because lift function is broken otherwise
struct identity
{
    template <class T>
    constexpr auto operator()( T&& t ) const noexcept
    {
        return std::forward<T>( t );
    }
};
