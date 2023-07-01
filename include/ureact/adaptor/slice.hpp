//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_SLICE_HPP
#define UREACT_ADAPTOR_SLICE_HPP

#include <type_traits>

#include <ureact/adaptor/filter.hpp>
#include <ureact/detail/take_drop_base.hpp>
#include <ureact/utility/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct SliceAdaptor : Adaptor
{
    /*!
	 * @brief Keeps given range (begin through end-1) of elements from the source stream
	 *
	 *  Semantically equivalent of proposed std::ranges::views::slice
	 */
    template <typename E, typename N, class = std::enable_if_t<std::is_integral_v<N>>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<E>& source, const N begin, const dont_deduce<N> end ) const
    {
        assert( begin >= 0 );
        assert( end >= begin );
        return filter( source,                                   //
            [begin = countdown( begin ), end = countdown( end )] //
            ( const auto& ) mutable {                            //
                return end-- && !begin--; // take "end" elements, but skip "begin" of them
            } );
    }

    template <typename N, class = std::enable_if_t<std::is_integral_v<N>>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const N begin, const dont_deduce<N> end ) const
    {
        assert( begin >= 0 );
        assert( end >= begin );
        return make_partial<SliceAdaptor>( begin, end );
    }
};

} // namespace detail

/*!
 * @brief Keeps given range (begin through end-1) of elements from the source stream
 *
 *  Semantically equivalent of proposed std::ranges::views::slice
 */
inline constexpr detail::SliceAdaptor slice;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_SLICE_HPP
