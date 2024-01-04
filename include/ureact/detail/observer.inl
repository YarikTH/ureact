//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_OBSERVER_INL
#define UREACT_DETAIL_OBSERVER_INL

#include <cassert>

#include <ureact/detail/defines.hpp>
#include <ureact/observer.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

UREACT_FUNC observer_internals::observer_internals( std::shared_ptr<observer_node> node )
    : m_node( std::move( node ) )
{}

UREACT_FUNC std::shared_ptr<observer_node>& observer_internals::get_node_ptr()
{
    return m_node;
}

UREACT_FUNC const std::shared_ptr<observer_node>& observer_internals::get_node_ptr() const
{
    return m_node;
}

UREACT_FUNC node_id observer_internals::get_node_id() const
{
    return m_node->get_node_id();
}

UREACT_FUNC react_graph& observer_internals::get_graph() const
{
    assert( m_node != nullptr && "Should be attached to a node" );
    return get_internals( m_node->get_context() ).get_graph();
}

} // namespace detail

UREACT_FUNC void observer::detach()
{
    assert( is_valid() );
    get_node_ptr()->detach_observer();
}

UREACT_FUNC bool observer::is_valid() const
{
    return this->get_node_ptr() != nullptr;
}

UREACT_FUNC observer::observer( std::shared_ptr<detail::observer_node> node )
    : observer_internals( std::move( node ) )
{}

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_OBSERVER_INL
