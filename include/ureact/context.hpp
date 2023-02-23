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

#include <ureact/detail/defines.hpp>
#include <ureact/detail/graph_impl.hpp>

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
    /*!
     * @brief Default construct @ref context
     */
    context() = default;

    UREACT_MAKE_COPYABLE( context );
    UREACT_MAKE_MOVABLE( context );

    /*!
     * @brief Equally compare with other @ref context
     */
    UREACT_WARN_UNUSED_RESULT friend bool operator==( const context& lhs, const context& rhs )
    {
        return &lhs.get_graph() == &rhs.get_graph();
    }

    /*!
     * @brief Equally compare with other @ref context
     */
    UREACT_WARN_UNUSED_RESULT friend bool operator!=( const context& lhs, const context& rhs )
    {
        return !( lhs == rhs );
    }

    /*!
     * @brief Return internals. Not intended to use in user code
     */
    UREACT_WARN_UNUSED_RESULT friend context_internals& get_internals( context& ctx )
    {
        return ctx;
    }

    /*!
     * @brief Return internals. Not intended to use in user code
     */
    UREACT_WARN_UNUSED_RESULT friend const context_internals& get_internals( const context& ctx )
    {
        return ctx;
    }
};

UREACT_END_NAMESPACE

#endif //UREACT_CONTEXT_HPP
