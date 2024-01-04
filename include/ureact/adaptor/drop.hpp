//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_DROP_HPP
#define UREACT_ADAPTOR_DROP_HPP

#include <cassert>
#include <type_traits>

#include <ureact/adaptor/detail/take_drop_base.hpp>
#include <ureact/adaptor/filter.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct DropAdaptor : TakeDropAdaptorBase<DropAdaptor>
{
    /*!
	 * @brief Skips first N elements from the source stream
	 *
	 *  Semantically equivalent of std::ranges::views::drop
	 */
    template <typename E, typename N, class = std::enable_if_t<std::is_integral_v<N>>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<E>& source, const N count ) const
    {
        assert( count >= 0 );
        return filter( source,                                //
            [i = countdown( count )]( const auto& ) mutable { //
                return !bool( i-- );
            } );
    }

    using TakeDropAdaptorBase::operator();
};

} // namespace detail

/*!
 * @brief Skips first N elements from the source stream
 *
 *  Semantically equivalent of std::ranges::views::drop
 */
inline constexpr detail::DropAdaptor drop;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_DROP_HPP
