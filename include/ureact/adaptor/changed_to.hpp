//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_CHANGED_TO_HPP
#define UREACT_ADAPTOR_CHANGED_TO_HPP

#include <ureact/adaptor/filter.hpp>
#include <ureact/adaptor/monitor.hpp>
#include <ureact/adaptor/unify.hpp>
#include <ureact/detail/adaptor.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct ChangedToAdaptor : Adaptor
{
    template <typename V, typename S = std::decay_t<V>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const signal<S>& target, V&& value ) const
    {
        return target | monitor | filter( [=]( const S& v ) { return v == value; } ) | unify;
    }

    template <typename V>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( V&& value ) const
    {
        return make_partial<ChangedToAdaptor>( std::forward<V>( value ) );
    }
};

} // namespace detail

/*!
 * @brief Emits unit when target signal was changed to value
 *  Creates a unit stream that emits when target is changed and 'target.get() == value'.
 *  V and S should be comparable with ==.
 */
inline constexpr detail::ChangedToAdaptor changed_to;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_CHANGED_TO_HPP
