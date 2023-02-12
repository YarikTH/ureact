//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_PULSE_HPP
#define UREACT_PULSE_HPP

#include <ureact/detail/adaptor.hpp>
#include <ureact/process.hpp>
#include <ureact/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct PulseAdaptor : Adaptor
{

    /*!
	 * @brief Emits the value of a target signal when an event is received
	 *
	 *  Creates an event stream that emits target.get() when receiving an event from trigger.
	 *  The values of the received events are irrelevant.
	 */
    template <typename S, typename E>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<E>& trigger, const signal<S>& target ) const
    {
        return process<S>( trigger,
            with( target ),
            []( event_range<E> range, event_emitter<S> out, const S& target_value ) {
                for( size_t i = 0, ie = range.size(); i < ie; ++i )
                    out << target_value;
            } );
    }

    /*!
	 * @brief Curried version of pulse()
	 */
    template <typename S>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const signal<S>& target ) const
    {
        return make_partial<PulseAdaptor>( target );
    }
};

} // namespace detail

inline constexpr detail::PulseAdaptor pulse;

UREACT_END_NAMESPACE

#endif // UREACT_PULSE_HPP
