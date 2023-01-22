//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_CONTEXT_HPP
#define UREACT_CONTEXT_HPP

#include <ureact/detail/base.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Core class that connects all reactive nodes together.
 *
 *  Each signal and node belongs to a single ureact context.
 *  Signals from different contexts can't interact with each other.
 */
class context final : protected detail::context_internals
{
public:
    UREACT_WARN_UNUSED_RESULT bool operator==( const context& rsh ) const
    {
        return this == &rsh;
    }

    UREACT_WARN_UNUSED_RESULT bool operator!=( const context& rsh ) const
    {
        return !( *this == rsh );
    }

private:
    friend class detail::node_base;

    /// Returns internals. Not intended to use in user code
    UREACT_WARN_UNUSED_RESULT friend context_internals& _get_internals( context& ctx )
    {
        return ctx;
    }
};

UREACT_END_NAMESPACE

#endif //UREACT_CONTEXT_HPP
