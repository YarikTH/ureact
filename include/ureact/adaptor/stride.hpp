//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_STRIDE_HPP
#define UREACT_ADAPTOR_STRIDE_HPP

#include <cassert>
#include <type_traits>

#include <ureact/adaptor/filter.hpp>
#include <ureact/detail/adaptor.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct StrideAdaptor : Adaptor
{
    /*!
	 * @brief Advances N elements from the source stream at a time
	 *
	 *  Semantically equivalent of std::ranges::views::stride
	 */
    template <typename E, typename N, class = std::enable_if_t<std::is_integral_v<N>>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<E>& source, const N n ) const
    {
        // based on https://stackoverflow.com/a/66839875/9323999
        assert( n >= 1 );
        return filter( source,                   //
            [i = -1, n]( const auto& ) mutable { //
                i = ( i + 1 ) % n;
                return !i;
            } );
    }

    template <typename N, class = std::enable_if_t<std::is_integral_v<N>>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const N n ) const
    {
        assert( n >= 1 );
        return make_partial<StrideAdaptor>( n );
    }
};

} // namespace detail

/*!
 * @brief Advances N elements from the source stream at a time
 *
 *  Semantically equivalent of std::ranges::views::stride
 */
inline constexpr detail::StrideAdaptor stride;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_STRIDE_HPP
