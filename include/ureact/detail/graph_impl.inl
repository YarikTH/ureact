//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_GRAPH_IMPL_INL
#define UREACT_DETAIL_GRAPH_IMPL_INL

#include <cassert>
#include <limits>
#include <memory>
#include <vector>

#include <ureact/detail/algorithm.hpp>
#include <ureact/detail/defines.hpp>
#include <ureact/detail/graph_impl.hpp>
#include <ureact/detail/graph_interface.hpp>
#include <ureact/detail/node_id_vector.hpp>
#include <ureact/detail/slot_map.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct null_react_graph_listener : react_graph_listener
{
    void node_is_registered( node_id ) override
    {}

    void node_is_queued_for_unregister( node_id ) override
    {}

    void node_is_unregistered( node_id ) override
    {}

    void node_is_attached( node_id, node_id ) override
    {}
    void node_is_detached( node_id, node_id ) override
    {}
};

class react_graph_impl : public react_graph
{
#if !defined( NDEBUG )
    bool m_is_locked = false;

    bool is_locked() const override
    {
        return m_is_locked;
    }

    void begin_external_callback() override
    {
        assert( !m_is_locked );
        m_is_locked = true;
    }

    void end_external_callback() override
    {
        assert( m_is_locked );
        m_is_locked = false;
    }
#endif
public:
    explicit react_graph_impl( std::shared_ptr<react_graph_listener> listener );
    ~react_graph_impl() override;

    node_id register_node() override;
    void register_node_ptr(
        node_id nodeId, const std::weak_ptr<reactive_node_interface>& nodePtr ) override;
    void unregister_node( node_id nodeId ) override;

    void attach_node( node_id nodeId, node_id parentId ) override;
    void detach_node( node_id nodeId, node_id parentId ) override;

    void push_input( node_id nodeId ) override;

    void start_transaction() override;
    void finish_transaction() override;

    [[nodiscard]] bool is_propagation_in_progress() const override
    {
        return m_propagation_is_in_progress;
    }

private:
    struct node_data
    {
        UREACT_MAKE_NONCOPYABLE( node_data );
        UREACT_MAKE_MOVABLE( node_data );

        node_data() = default;

        int level = 0;
        int new_level = 0;
        bool queued = false;

        std::weak_ptr<reactive_node_interface> node_ptr;

        node_id_vector successors;
    };

    class topological_queue
    {
    public:
        using value_type = node_id;

        topological_queue() = default;

        void push( const value_type& value, const int level )
        {
            m_queue_data.emplace_back( value, level );
        }

        bool fetch_next();

        UREACT_WARN_UNUSED_RESULT const std::vector<value_type>& next_values() const
        {
            return m_next_data;
        }

        UREACT_WARN_UNUSED_RESULT bool empty() const
        {
            return m_queue_data.empty() && m_next_data.empty();
        }

    private:
        using entry = std::pair<value_type, int>;

        std::vector<value_type> m_next_data;
        std::vector<entry> m_queue_data;
    };

    UREACT_WARN_UNUSED_RESULT static node_id::context_id_type create_context_id();

    UREACT_WARN_UNUSED_RESULT bool can_unregister_node() const;

    void propagate();

    void recalculate_successor_levels( node_data& parentNode );

    void schedule_node( node_id nodeId );

    void re_schedule_node( node_id nodeId );

    void schedule_successors( node_data& parentNode );

    void propagate_node_change( node_id nodeId );

    void finalize_changed_nodes();

    void unregister_queued_nodes();

    std::shared_ptr<react_graph_listener> m_listener;

    node_id::context_id_type m_id = create_context_id();

    slot_map<node_data> m_node_data;

    topological_queue m_scheduled_nodes;

    int m_transaction_level = 0;

    bool m_propagation_is_in_progress = false;

    // local to propagate. Moved here to not reallocate
    node_id_vector m_changed_nodes;

    node_id_vector m_nodes_queued_for_unregister;
};

UREACT_FUNC react_graph_impl::react_graph_impl( std::shared_ptr<react_graph_listener> listener )
    : m_listener( std::move( listener ) )
{}

UREACT_FUNC react_graph_impl::~react_graph_impl()
{
    assert( m_node_data.empty() );
    assert( m_scheduled_nodes.empty() );
    assert( m_transaction_level == 0 );
    assert( m_propagation_is_in_progress == false );
    assert( m_changed_nodes.empty() );
    assert( m_nodes_queued_for_unregister.empty() );
}

UREACT_FUNC node_id react_graph_impl::register_node()
{
    return node_id{ m_id, m_node_data.emplace() };
}

UREACT_FUNC void react_graph_impl::register_node_ptr(
    const node_id nodeId, const std::weak_ptr<reactive_node_interface>& nodePtr )
{
    assert( nodeId.context_id() == m_id );
    assert( nodePtr.use_count() > 0 );

    node_data& node = m_node_data[nodeId];
    node.node_ptr = nodePtr;
    m_listener->node_is_registered( nodeId );
}

UREACT_FUNC void react_graph_impl::unregister_node( const node_id nodeId )
{
    assert( nodeId.context_id() == m_id );
    assert( m_node_data[nodeId].successors.empty() );

    if( can_unregister_node() )
    {
        m_node_data.erase( nodeId );
        m_listener->node_is_unregistered( nodeId );
    }
    else
    {
        m_nodes_queued_for_unregister.add( nodeId );
        m_listener->node_is_queued_for_unregister( nodeId );
    }
}

UREACT_FUNC void react_graph_impl::attach_node( const node_id nodeId, const node_id parentId )
{
    assert( nodeId.context_id() == m_id );
    assert( parentId.context_id() == m_id );

    node_data& node = m_node_data[nodeId];
    node_data& parent = m_node_data[parentId];

    parent.successors.add( nodeId );

    node.level = std::max( node.level, parent.level + 1 );

    m_listener->node_is_attached( nodeId, parentId );
}

UREACT_FUNC void react_graph_impl::detach_node( const node_id nodeId, const node_id parentId )
{
    assert( nodeId.context_id() == m_id );
    assert( parentId.context_id() == m_id );

    node_data& parent = m_node_data[parentId];
    node_id_vector& successors = parent.successors;

    successors.remove( nodeId );

    m_listener->node_is_detached( nodeId, parentId );
}

UREACT_FUNC void react_graph_impl::push_input( const node_id nodeId )
{
    assert( !m_propagation_is_in_progress );

    schedule_node( nodeId );

    if( m_transaction_level == 0 )
        propagate();
}

UREACT_FUNC void react_graph_impl::start_transaction()
{
    assert( !m_propagation_is_in_progress );

    ++m_transaction_level;
}

UREACT_FUNC void react_graph_impl::finish_transaction()
{
    assert( !m_propagation_is_in_progress );
    assert( m_transaction_level > 0 );

    --m_transaction_level;

    if( m_transaction_level == 0 )
        propagate();
}

UREACT_FUNC node_id::context_id_type react_graph_impl::create_context_id()
{
    static node_id::context_id_type s_next_id = 1u;
    return s_next_id++;
}

UREACT_FUNC bool react_graph_impl::can_unregister_node() const
{
    return m_transaction_level == 0 && !m_propagation_is_in_progress;
}

UREACT_FUNC void react_graph_impl::propagate()
{
    m_propagation_is_in_progress = true;

    while( m_scheduled_nodes.fetch_next() )
        for( const node_id nodeId : m_scheduled_nodes.next_values() )
            propagate_node_change( nodeId );

    finalize_changed_nodes();

    m_propagation_is_in_progress = false;

    unregister_queued_nodes();
}

UREACT_FUNC void react_graph_impl::recalculate_successor_levels( node_data& parentNode )
{
    for( const node_id successorId : parentNode.successors )
    {
        node_data& successorNode = m_node_data[successorId];
        successorNode.new_level = std::max( successorNode.new_level, parentNode.level + 1 );
    }
}

UREACT_FUNC void react_graph_impl::schedule_node( const node_id nodeId )
{
    node_data& node = m_node_data[nodeId];

    if( !node.queued )
    {
        node.queued = true;
        m_scheduled_nodes.push( nodeId, node.level );
    }
}

UREACT_FUNC void react_graph_impl::re_schedule_node( const node_id nodeId )
{
    node_data& node = m_node_data[nodeId];
    recalculate_successor_levels( node );
    m_scheduled_nodes.push( nodeId, node.level );
}

UREACT_FUNC void react_graph_impl::schedule_successors( node_data& parentNode )
{
    for( const node_id successorId : parentNode.successors )
        schedule_node( successorId );
}

UREACT_FUNC void react_graph_impl::propagate_node_change( const node_id nodeId )
{
    node_data& node = m_node_data[nodeId];
    if( std::shared_ptr<reactive_node_interface> nodePtr = node.node_ptr.lock() )
    {
        // A predecessor of this node has shifted to a lower level?
        if( node.level < node.new_level )
        {
            node.level = node.new_level;
            re_schedule_node( nodeId );
            return;
        }

        const update_result result = nodePtr->update();

        // Topology changed?
        if( result == update_result::shifted )
        {
            re_schedule_node( nodeId );
            return;
        }

        if( result == update_result::changed )
        {
            m_changed_nodes.add( nodeId );
            schedule_successors( node );
        }
    }

    node.queued = false;
}

UREACT_FUNC void react_graph_impl::finalize_changed_nodes()
{
    // Cleanup buffers in changed nodes etc
    for( const node_id nodeId : m_changed_nodes )
    {
        node_data& node = m_node_data[nodeId];
        if( std::shared_ptr<reactive_node_interface> nodePtr = node.node_ptr.lock() )
        {
            nodePtr->finalize();
        }
    }
    m_changed_nodes.clear();
}

UREACT_FUNC void react_graph_impl::unregister_queued_nodes()
{
    assert( !m_propagation_is_in_progress );

    for( const node_id nodeId : m_nodes_queued_for_unregister )
        unregister_node( nodeId );
    m_nodes_queued_for_unregister.clear();
}

UREACT_FUNC bool react_graph_impl::topological_queue::fetch_next()
{
    // Throw away previous values
    m_next_data.clear();

    // Find min level of nodes in queue data
    auto minimal_level = std::numeric_limits<int>::max();
    for( const entry& e : m_queue_data )
        minimal_level = std::min( minimal_level, e.second );

    // Swap entries with min level to the end
    const auto p = detail::partition( m_queue_data.begin(),
        m_queue_data.end(),
        [minimal_level]( const entry& e ) { return e.second != minimal_level; } );

    // Reserve once to avoid multiple re-allocations
    const auto to_reserve = static_cast<size_t>( std::distance( p, m_queue_data.end() ) );
    m_next_data.reserve( to_reserve );

    // Move min level values to next data
    for( auto it = p, ite = m_queue_data.end(); it != ite; ++it )
        m_next_data.push_back( it->first );

    // Truncate moved entries
    const auto to_resize = static_cast<size_t>( std::distance( m_queue_data.begin(), p ) );
    m_queue_data.resize( to_resize );

    return !m_next_data.empty();
}

UREACT_FUNC std::shared_ptr<react_graph> make_react_graph(
    std::shared_ptr<react_graph_listener> listener )
{
    return std::make_shared<react_graph_impl>(
        listener ? std::move( listener ) //
                 : std::make_shared<null_react_graph_listener>() );
}

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_GRAPH_IMPL_INL
