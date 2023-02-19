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
    using value_type = size_t;

    node_id() = default;

    explicit node_id( value_type id )
        : m_id( id )
    {}

    operator value_type() // NOLINT
    {
        return m_id;
    }

    bool operator==( node_id other ) const noexcept
    {
        return m_id == other.m_id;
    }

    bool operator!=( node_id other ) const noexcept
    {
        return m_id != other.m_id;
    }

private:
    value_type m_id = -1;
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

class observer_interface
{
public:
    virtual ~observer_interface() = default;

    virtual void unregister_self() = 0;

private:
    virtual void detach_observer() = 0;

    friend class observable;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_GRAPH_INTERFACE_HPP
