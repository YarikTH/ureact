//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_SNAPSHOT_HPP
#define UREACT_ADAPTOR_SNAPSHOT_HPP

#include <ureact/adaptor/fold.hpp>
#include <ureact/detail/adaptor.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct SnapshotAdaptor : Adaptor
{
    /*!
	 * @brief Sets the signal value to the value of a target signal when an event is received
	 *
	 *  Creates a signal with value v = target.get().
	 *  The value is set on construction and updated only when receiving an event from trigger
	 */
    template <typename S, typename E>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<E>& trigger, const signal<S>& target ) const
    {
        return fold( trigger,
            target.get(),
            with( target ),
            []( event_range<E>, const S&, const S& value ) { //
                return value;
            } );
    }

    /*!
	 * @brief Curried version of snapshot()
	 */
    template <typename S>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const signal<S>& target ) const
    {
        return make_partial<SnapshotAdaptor>( target );
    }
};

} // namespace detail

inline constexpr detail::SnapshotAdaptor snapshot;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_SNAPSHOT_HPP
