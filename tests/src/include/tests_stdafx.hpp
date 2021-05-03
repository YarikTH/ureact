#pragma once

#include <doctest.h>
#include <string>
#include <vector>
#include <sstream>

namespace std // NOLINT
{

template <class T, class Allocator>
doctest::String toString( const std::vector<T, Allocator>& value )
{
    std::stringstream ss;

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
