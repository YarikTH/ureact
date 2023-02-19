//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_NODE_BASE_HPP
#define UREACT_DETAIL_NODE_BASE_HPP

#include <ureact/detail/base.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

class node_base : public reactive_node_interface
{
public:
    explicit node_base( context& context )
        : m_context( context )
    {
        assert( !get_graph().is_locked() && "Can't create node from callback" );
        m_id = get_graph().register_node( this );
    }

    ~node_base() override
    {
        get_graph().unregister_node( m_id );
    }

    UREACT_WARN_UNUSED_RESULT node_id get_node_id() const
    {
        return m_id;
    }

    UREACT_WARN_UNUSED_RESULT context& get_context() const
    {
        return m_context;
    }

    UREACT_WARN_UNUSED_RESULT react_graph& get_graph()
    {
        return _get_internals( m_context ).get_graph();
    }

    UREACT_WARN_UNUSED_RESULT const react_graph& get_graph() const
    {
        return _get_internals( m_context ).get_graph();
    }

protected:
    void attach_to( node_id parentId )
    {
        get_graph().attach_node( m_id, parentId );
    }

    void detach_from( node_id parentId )
    {
        get_graph().detach_node( m_id, parentId );
    }

private:
    UREACT_MAKE_NONCOPYABLE( node_base );

    context& m_context;

    node_id m_id;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_NODE_BASE_HPP
