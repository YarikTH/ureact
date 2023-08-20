//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#if TRANSFORM_VERSION == 1
#    include <ureact/adaptor/transform.hpp>
#    define ADAPTOR_NAME transform
#endif

#ifndef INCLUDE_ONLY

#    include <cassert>
#    include <utility>

using namespace ureact;
using namespace ureact::default_context;

auto make_mult_by_n( int N )
{
    return [N]( int value ) { return value * N; };
}

template <size_t N>
void test_function()
{
    auto src = make_source<int>();
    assert( src.is_valid() );

#    ifndef MAKE_SOURCE_ONLY

    const auto func = []() {
        if constexpr( UNIQUE_FUNC )
            return []( int value ) { return value * N; };
        else
            return make_mult_by_n( N );
    }();

    auto x = [&]() {
        if constexpr( USE_PIPE )
            return src | ADAPTOR_NAME( func );
        else
            return ADAPTOR_NAME( src, func );
    }();

    assert( x.is_valid() );

#    endif
}

template <std::size_t... I>
void make_test_functions_impl( std::integer_sequence<std::size_t, I...> )
{
    ( test_function<I>(), ... );
}

void make_test_functions()
{
    make_test_functions_impl( std::make_index_sequence<COPY_COUNT>{} );
}

#endif

int main()
{}
