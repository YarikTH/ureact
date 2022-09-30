//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_REACTIVE_REF_HPP
#define UREACT_REACTIVE_REF_HPP

#include "flatten.hpp"
#include "lift.hpp"

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Utility to flatten public signal attribute of class pointed be reference
 *
 *  For example we have a class Foo with a public signal bar: struct Foo{ signal<int> bar; };
 *  Also, we have signal that points to this class by pointer: signal<Foo*> bar
 *  This utility receives a signal pointer bar and attribute pointer &Foo::bar and flattens it to signal<int> foobar
 */
template <typename Signal,
    typename InF,
    class = std::enable_if_t<is_signal_v<std::decay_t<Signal>>>>
UREACT_WARN_UNUSED_RESULT auto reactive_ref( Signal&& outer, InF&& func )
{
    using S = typename std::decay_t<Signal>::value_t;
    using F = std::decay_t<InF>;
    using R = std::invoke_result_t<F, S>;
    using DecayedR = detail::decay_input_t<std::decay_t<R>>;
    return flatten( lift<DecayedR>( std::forward<Signal>( outer ), std::forward<InF>( func ) ) );
}

UREACT_END_NAMESPACE

#endif // UREACT_REACTIVE_REF_HPP
