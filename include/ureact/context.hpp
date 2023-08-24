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

#include <memory>

#include <ureact/detail/defines.hpp>
#include <ureact/detail/graph_impl.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

class context_internals
{
public:
    explicit context_internals( std::shared_ptr<react_graph> graph = make_react_graph() );

    UREACT_WARN_UNUSED_RESULT react_graph& get_graph();
    UREACT_WARN_UNUSED_RESULT const react_graph& get_graph() const;

private:
    std::shared_ptr<react_graph> m_graph_ptr;
};

} // namespace detail

class context;

namespace default_context
{

/**
 * @brief Return default context
 * Default contexts are thread_local
 */
context get();

} // namespace default_context

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

private:
    friend context default_context::get();

    /*!
     * @brief Construct @ref context from given react_graph
     */
    explicit context( std::shared_ptr<detail::react_graph> graph );
};

UREACT_END_NAMESPACE

#if UREACT_HEADER_ONLY
#    include <ureact/detail/context.inl>
#endif

#endif //UREACT_CONTEXT_HPP
