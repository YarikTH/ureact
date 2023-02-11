//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_FILTER_HPP
#define UREACT_FILTER_HPP

#include <ureact/detail/closure.hpp>
#include <ureact/process.hpp>
#include <ureact/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Create a new event stream that filters events from other stream
 *
 *  For every event e in source, emit e if pred(e, deps...) == true.
 *  Synchronized values of signals in dep_pack are passed to op as additional arguments.
 *
 *  The signature of pred should be equivalent to:
 *  * bool pred(const E&, const Deps& ...)
 *
 *  Semantically equivalent of ranges::filter
 *
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename E, typename Pred, typename... DepValues>
UREACT_WARN_UNUSED_RESULT auto filter(
    const events<E>& source, const signal_pack<DepValues...>& dep_pack, Pred&& pred ) -> events<E>
{
    return detail::process_impl<E>( source,
        dep_pack, //
        [pred = std::forward<Pred>( pred )](
            event_range<E> range, event_emitter<E> out, const auto... deps ) mutable {
            for( const auto& e : range )
                if( std::invoke( pred, e, deps... ) )
                    out << e;
        } );
}

/*!
 * @brief Curried version of filter(const events<E>& source, const signal_pack<DepValues...>& dep_pack, Pred&& pred)
 */
template <typename Pred, typename... DepValues>
UREACT_WARN_UNUSED_RESULT auto filter( const signal_pack<DepValues...>& dep_pack, Pred&& pred )
{
    return detail::closure{
        [dep_pack = dep_pack, pred = std::forward<Pred>( pred )]( auto&& source ) {
            using arg_t = decltype( source );
            static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
            return filter( std::forward<arg_t>( source ), dep_pack, pred );
        } };
}

/*!
 * @brief Create a new event stream that filters events from other stream
 *
 *  Version without synchronization with additional signals
 *
 *  See filter(const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred)
 */
template <typename E, typename Pred>
UREACT_WARN_UNUSED_RESULT auto filter( const events<E>& source, Pred&& pred ) -> events<E>
{
    return filter( source, signal_pack<>(), std::forward<Pred>( pred ) );
}

/*!
 * @brief Curried version of filter(const events<E>& source, Pred&& pred)
 */
template <typename Pred>
UREACT_WARN_UNUSED_RESULT auto filter( Pred&& pred )
{
    return detail::closure{ [pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return filter( std::forward<arg_t>( source ), pred );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_FILTER_HPP
