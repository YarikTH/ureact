//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_SINK_HPP
#define UREACT_SINK_HPP

#include <ureact/closure.hpp>
#include <ureact/ureact.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Gets the reference to the destination and assign value from the left to it
 *
 *  Forward source so it can be chained further. Works similar to "tap"
 */
template <class Dst, class = std::enable_if_t<is_reactive_v<std::decay_t<Dst>>>>
UREACT_WARN_UNUSED_RESULT auto sink( Dst& dst )
{
    // TODO: propagate [[nodiscard(false)]] somehow
    return closure{ [&dst]( auto&& source ) -> decltype( auto ) {
        using arg_t = decltype( source );

        if constexpr( is_signal_v<std::decay_t<Dst>> )
        {
            static_assert( is_signal_v<std::decay_t<arg_t>>, "Signal type is required" );
        }
        else if constexpr( is_event_v<std::decay_t<Dst>> )
        {
            static_assert( is_event_v<std::decay_t<arg_t>>, "Events type is required" );
        }
        else if constexpr( is_observer_v<std::decay_t<Dst>> )
        {
            static_assert( is_observer_v<std::decay_t<arg_t>>, "Observer type is required" );
        }
        else
        {
            static_assert( detail::always_false<Dst>, "Unsupported Dst type" );
        }

        dst = std::forward<arg_t>( source );

        return dst;
    } };
}

UREACT_END_NAMESPACE

#endif //UREACT_SINK_HPP
