//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_MONITOR_CHANGE_HPP
#define UREACT_ADAPTOR_MONITOR_CHANGE_HPP

#include <ureact/adaptor/monitor.hpp>
#include <ureact/adaptor/transform.hpp>
#include <ureact/detail/adaptor.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct MonitorChangeClosure : adaptor_closure
{
    /*!
	 * @brief Emits pairs of value changes of signal as events
	 *
	 *  When target changes, emit the new value 'e = std::pair(old_value, target.get())'.
	 */
    template <typename S>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const signal<S>& target ) const
    {
        return target //
             | monitor
             | transform(
                 [memento = get_internals( target ).value_ref()]( const S& new_value ) mutable //
                 {
                     S old_value = memento;
                     memento = new_value;
                     return std::make_pair( std::move( old_value ), new_value );
                 } );
    }
};

} // namespace detail


inline constexpr detail::MonitorChangeClosure monitor_change;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_MONITOR_CHANGE_HPP
