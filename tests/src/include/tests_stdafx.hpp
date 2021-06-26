//
//         Copyright (C) 2020-2021 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <sstream>
#include <string>
#include <vector>

#include <doctest.h>

namespace std // NOLINT
{

template <class T, class Allocator>
doctest::String toString( const std::vector<T, Allocator>& value )
{
    std::ostringstream ss;

    ss << "[";
    for( auto it = value.begin(), ite = value.end(); it != ite; ++it )
    {
        ss << *it;
        if( it + 1 != ite )
        {
            ss << ", ";
        }
    }
    ss << "]";

    return ss.str().c_str();
}

} // namespace std
