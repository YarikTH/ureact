//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_TRANSFORM_HPP
#define UREACT_TRANSFORM_HPP

#include "closure.hpp"
#include "process.hpp"

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Create a new event stream that transforms events from other stream
 *
 *  For every event e in source, emit t = func(e, deps...).
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of func should be equivalent to:
 *  * T func(const E&, const Deps& ...)
 *
 *  Semantically equivalent of ranges::transform
 *
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename InE,
    typename F,
    typename... Deps,
    typename OutE = std::invoke_result_t<F, InE, Deps...>>
UREACT_WARN_UNUSED_RESULT auto transform(
    const events<InE>& source, const signal_pack<Deps...>& dep_pack, F&& func ) -> events<OutE>
{
    return detail::process_impl<OutE>( source,
        dep_pack, //
        [func = std::forward<F>( func )](
            event_range<InE> range, event_emitter<OutE> out, const auto... deps ) mutable {
            for( const auto& e : range )
                out << std::invoke( func, e, deps... );
        } );
}

/*!
 * @brief Curried version of transform(const events<InE>& source, const signal_pack<Deps...>& dep_pack, F&& func)
 */
template <typename F, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto transform( const signal_pack<Deps...>& dep_pack, F&& func )
{
    return closure{ [deps = dep_pack.store(), func = std::forward<F>( func )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return transform( std::forward<arg_t>( source ), signal_pack<Deps...>{ deps }, func );
    } };
}

/*!
 * @brief Create a new event stream that transforms events from other stream
 *
 *  Version without synchronization with additional signals
 *
 *  See transform(const events<in_t>& source, const signal_pack<Deps...>& dep_pack, F&& func)
 */
template <typename InE, typename F, typename OutE = std::invoke_result_t<F, InE>>
UREACT_WARN_UNUSED_RESULT auto transform( const events<InE>& source, F&& func ) -> events<OutE>
{
    return transform( source, signal_pack<>(), std::forward<F>( func ) );
}

/*!
 * @brief Curried version of transform(const events<InE>& source, F&& func)
 */
template <typename F>
UREACT_WARN_UNUSED_RESULT auto transform( F&& func )
{
    return closure{ [func = std::forward<F>( func )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return transform( std::forward<arg_t>( source ), func );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_TRANSFORM_HPP
