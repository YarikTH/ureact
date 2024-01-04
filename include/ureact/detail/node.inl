//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_NODE_INL
#define UREACT_DETAIL_NODE_INL

#include <cassert>

#include <ureact/detail/defines.hpp>
#include <ureact/detail/node.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

UREACT_FUNC node::node( context context )
    : m_context( std::move( context ) )
{
    assert( !get_graph().is_locked() && "Can't create node from callback" );
    m_id = get_graph().register_node();
}

UREACT_FUNC node::~node()
{
    detach_from_all();
    get_graph().unregister_node( m_id );
}

UREACT_FUNC react_graph& node::get_graph()
{
    return get_internals( m_context ).get_graph();
}

UREACT_FUNC const react_graph& node::get_graph() const
{
    return get_internals( m_context ).get_graph();
}

UREACT_FUNC void node::attach_to( node_id parentId )
{
    m_parents.add( parentId );
    get_graph().attach_node( m_id, parentId );
}

UREACT_FUNC void node::detach_from( node_id parentId )
{
    get_graph().detach_node( m_id, parentId );
    m_parents.remove( parentId );
}

UREACT_FUNC void node::detach_from_all()
{
    for( node_id parentId : m_parents )
        get_graph().detach_node( m_id, parentId );
    m_parents.clear();
}

} // namespace detail

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_NODE_INL
