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

#include <memory>
#include <utility>

#include <ureact/context.hpp>
#include <ureact/detail/graph_impl.hpp>
#include <ureact/detail/graph_interface.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename Ret, typename Node, typename... Args>
Ret create_wrapped_node( Args&&... args )
{
    return Ret{ std::make_shared<Node>( std::forward<Args>( args )... ) };
}

class node_base : public reactive_node_interface
{
public:
    explicit node_base( context context )
        : m_context( std::move( context ) )
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

    UREACT_WARN_UNUSED_RESULT context& get_context()
    {
        return m_context;
    }

    UREACT_WARN_UNUSED_RESULT const context& get_context() const
    {
        return m_context;
    }

    // TODO protected:
    UREACT_WARN_UNUSED_RESULT react_graph& get_graph()
    {
        return get_internals( m_context ).get_graph();
    }

    UREACT_WARN_UNUSED_RESULT const react_graph& get_graph() const
    {
        return get_internals( m_context ).get_graph();
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

    context m_context{};

    node_id m_id;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_NODE_BASE_HPP
