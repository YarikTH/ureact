//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_GRAPH_INTERFACE_HPP
#define UREACT_DETAIL_GRAPH_INTERFACE_HPP

#include <cstddef>

#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

class node_id
{
public:
    using context_id_type = size_t;
    using value_type = size_t;

    node_id() = default;

    node_id( context_id_type context_id, value_type id )
        : m_context_id( context_id )
        , m_id( id )
    {}

    context_id_type context_id() const
    {
        return m_context_id;
    }

    operator value_type() const // NOLINT
    {
        return m_id;
    }

    bool operator==( node_id other ) const noexcept
    {
        return m_context_id == other.m_context_id && m_id == other.m_id;
    }

    bool operator!=( node_id other ) const noexcept
    {
        return !operator==( other );
    }

private:
    context_id_type m_context_id = context_id_type( -1 );
    value_type m_id = value_type( -1 );
};

enum class update_result
{
    unchanged,
    changed,
    shifted
};

struct reactive_node_interface
{
    virtual ~reactive_node_interface() = default;

    UREACT_WARN_UNUSED_RESULT virtual update_result update() = 0;

    /// Called after change propagation on changed nodes
    virtual void finalize()
    {}
};

struct observer_interface
{
    virtual ~observer_interface() = default;

    virtual void detach_observer() = 0;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_GRAPH_INTERFACE_HPP
