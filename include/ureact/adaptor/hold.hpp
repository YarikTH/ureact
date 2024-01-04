//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_HOLD_HPP
#define UREACT_ADAPTOR_HOLD_HPP

#include <ureact/adaptor/fold.hpp>
#include <ureact/detail/adaptor.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct HoldAdaptor : adaptor
{
    /*!
	 * @brief Holds the most recent event in a signal
	 *
	 *  Creates a @ref signal with an initial value v = init.
	 *  For received event values e1, e2, ... eN in events, it is updated to v = eN.
	 */
    template <typename V, typename E>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<E>& source, V&& init ) const
    {
        return fold( source,
            std::forward<V>( init ),                  //
            []( event_range<E> range, const auto& ) { //
                return *range.rbegin();
            } );
    }

    /*!
	 * @brief Curried version of hold()
	 */
    template <typename V>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( V&& init ) const
    {
        return make_partial<HoldAdaptor>( std::forward<V>( init ) );
    }
};

} // namespace detail

inline constexpr detail::HoldAdaptor hold;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_HOLD_HPP
