//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_CHANGED_HPP
#define UREACT_CHANGED_HPP

#include <ureact/detail/closure.hpp>
#include <ureact/filter.hpp>
#include <ureact/monitor.hpp>
#include <ureact/type_traits.hpp>
#include <ureact/unify.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Emits unit when target signal was changed
 *
 *  Creates a unit stream that emits when target is changed.
 */
//template <typename S>
//UREACT_WARN_UNUSED_RESULT auto changed( const signal<S>& target ) -> events<unit>
//{
//    return monitor( target ) | unify();
//}

/*!
 * @brief Curried version of changed(const signal<S>& target)
 */
//UREACT_WARN_UNUSED_RESULT inline auto changed()
//{
//    return detail::closure{ []( auto&& source ) {
//        using arg_t = decltype( source );
//        static_assert( is_signal_v<std::decay_t<arg_t>>, "Signal type is required" );
//        return changed( std::forward<arg_t>( source ) );
//    } };
//}

/*!
 * @brief Emits unit when target signal was changed to value
 *  Creates a unit stream that emits when target is changed and 'target.get() == value'.
 *  V and S should be comparable with ==.
 */
//template <typename V, typename S = std::decay_t<V>>
//UREACT_WARN_UNUSED_RESULT auto changed_to( const signal<S>& target, V&& value ) -> events<unit>
//{
//    return monitor( target ) | filter( [=]( const S& v ) { return v == value; } ) | unify();
//}

/*!
 * @brief Curried version of changed_to(const signal<S>& target, V&& value)
 */
//template <typename V, typename S = std::decay_t<V>>
//UREACT_WARN_UNUSED_RESULT inline auto changed_to( V&& value )
//{
//    return detail::closure{ [value = std::forward<V>( value )]( auto&& source ) {
//        using arg_t = decltype( source );
//        static_assert( is_signal_v<std::decay_t<arg_t>>, "Signal type is required" );
//        return changed_to( std::forward<arg_t>( source ), std::move( value ) );
//    } };
//}

UREACT_END_NAMESPACE

#endif // UREACT_CHANGED_HPP
