//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

// List of headers that are used on ureact
// headers that I assume have small impact on the compilation time are ouside of
// conditional compilation
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

#if defined( INCLUDE_TUPLE ) or defined( INCLUDE_ALL )
#    include <tuple>
#endif
#if defined( INCLUDE_MEMORY ) or defined( INCLUDE_ALL )
#    include <memory>
#endif
#if defined( INCLUDE_VECTOR ) or defined( INCLUDE_ALL )
#    include <vector>
#endif
#if defined( INCLUDE_ARRAY )
#    include <array> // is used for adjastment
#endif
#if defined( INCLUDE_DEQUE )
#    include <deque> // is used for zip
#endif
#if defined( INCLUDE_FUNCTIONAL )
#    include <functional> // was used for signal.hpp, is used for a lot of adaptors
#endif
#if defined( INCLUDE_ITERATOR )
#    include <iterator> // was used for ureact's <algorithm> replacement
#endif
#if defined( INCLUDE_ALGORITHM )
#    include <algorithm> // is used for graph_impl.hpp, but ureact provides replacement
#endif

int main()
{}
