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
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <ureact/detail/algorithm.hpp>
#include <ureact/detail/defines.hpp>
#include <ureact/detail/graph_interface.hpp>
#include <ureact/detail/slot_map.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

#if !defined( NDEBUG )
/// Utility class to check if callbacks passed in lift(), process() etc
/// are used properly
class callback_sanitizer
{
public:
    /// Return if external callback is in progress
    UREACT_WARN_UNUSED_RESULT bool is_locked() const
    {
        return m_is_locked;
    }

    /// Marks begin of an external callback
    void begin_external_callback()
    {
        assert( !m_is_locked );
        m_is_locked = true;
    }

    /// Marks end of an external callback
    void end_external_callback()
    {
        assert( m_is_locked );
        m_is_locked = false;
    }

    /// Marks a place where an external callback is called
    struct guard
    {
        callback_sanitizer& self;

        explicit guard( callback_sanitizer& self )
            : self( self )
        {
            self.begin_external_callback();
        }

        ~guard()
        {
            self.end_external_callback();
        }

        UREACT_MAKE_NONCOPYABLE( guard );
        UREACT_MAKE_NONMOVABLE( guard );
    };

private:
    bool m_is_locked = false;
};

#    define UREACT_CALLBACK_GUARD( _SELF_ ) callback_sanitizer::guard _( _SELF_ )
#else
#    define UREACT_CALLBACK_GUARD( _SELF_ )                                                        \
        do                                                                                         \
        {                                                                                          \
        } while( false )
#endif

class react_graph
#if !defined( NDEBUG )
    : public callback_sanitizer
#endif
{
public:
    react_graph() = default;
    ~react_graph();

    node_id register_node();
    void register_node_ptr( node_id nodeId, const std::weak_ptr<reactive_node_interface>& nodePtr );
    void unregister_node( node_id nodeId );

    void attach_node( node_id nodeId, node_id parentId );
    void detach_node( node_id nodeId, node_id parentId );

    void push_input( node_id nodeId );

    void start_transaction();
    void finish_transaction();

    [[nodiscard]] bool is_propagation_in_progress() const
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

    node_id::context_id_type m_id = create_context_id();

    slot_map<node_data> m_node_data;

    topological_queue m_scheduled_nodes;

    int m_transaction_level = 0;

    bool m_propagation_is_in_progress = false;

    // local to propagate. Moved here to not reallocate
    node_id_vector m_changed_nodes;

    node_id_vector m_nodes_queued_for_unregister;
};

inline react_graph::~react_graph()
{
    assert( m_node_data.empty() );
    assert( m_scheduled_nodes.empty() );
    assert( m_transaction_level == 0 );
    assert( m_propagation_is_in_progress == false );
    assert( m_changed_nodes.empty() );
    assert( m_nodes_queued_for_unregister.empty() );
}

inline node_id react_graph::register_node()
{
    return node_id{ m_id, m_node_data.emplace() };
}

inline void react_graph::register_node_ptr(
    const node_id nodeId, const std::weak_ptr<reactive_node_interface>& nodePtr )
{
    assert( nodeId.context_id() == m_id );
    assert( nodePtr.use_count() > 0 );

    node_data& node = m_node_data[nodeId];
    node.node_ptr = nodePtr;
}

inline void react_graph::unregister_node( const node_id nodeId )
{
    assert( nodeId.context_id() == m_id );
    assert( m_node_data[nodeId].successors.empty() );

    if( can_unregister_node() )
        m_node_data.erase( nodeId );
    else
        m_nodes_queued_for_unregister.add( nodeId );
}

inline void react_graph::attach_node( const node_id nodeId, const node_id parentId )
{
    assert( nodeId.context_id() == m_id );
    assert( parentId.context_id() == m_id );

    node_data& node = m_node_data[nodeId];
    node_data& parent = m_node_data[parentId];

    parent.successors.add( nodeId );

    node.level = std::max( node.level, parent.level + 1 );
}

inline void react_graph::detach_node( const node_id nodeId, const node_id parentId )
{
    assert( nodeId.context_id() == m_id );
    assert( parentId.context_id() == m_id );

    node_data& parent = m_node_data[parentId];
    node_id_vector& successors = parent.successors;

    successors.remove( nodeId );
}

inline void react_graph::push_input( const node_id nodeId )
{
    assert( !m_propagation_is_in_progress );

    schedule_node( nodeId );

    if( m_transaction_level == 0 )
        propagate();
}

inline void react_graph::start_transaction()
{
    assert( !m_propagation_is_in_progress );

    ++m_transaction_level;
}

inline void react_graph::finish_transaction()
{
    assert( !m_propagation_is_in_progress );
    assert( m_transaction_level > 0 );

    --m_transaction_level;

    if( m_transaction_level == 0 )
        propagate();
}

inline node_id::context_id_type react_graph::create_context_id()
{
    static node_id::context_id_type s_next_id = 1u;
    return s_next_id++;
}

UREACT_WARN_UNUSED_RESULT inline bool react_graph::can_unregister_node() const
{
    return m_transaction_level == 0 && !m_propagation_is_in_progress;
}

inline void react_graph::propagate()
{
    m_propagation_is_in_progress = true;

    while( m_scheduled_nodes.fetch_next() )
        for( const node_id nodeId : m_scheduled_nodes.next_values() )
            propagate_node_change( nodeId );

    finalize_changed_nodes();

    m_propagation_is_in_progress = false;

    unregister_queued_nodes();
}

inline void react_graph::recalculate_successor_levels( node_data& parentNode )
{
    for( const node_id successorId : parentNode.successors )
    {
        node_data& successorNode = m_node_data[successorId];
        successorNode.new_level = std::max( successorNode.new_level, parentNode.level + 1 );
    }
}

inline void react_graph::schedule_node( const node_id nodeId )
{
    node_data& node = m_node_data[nodeId];

    if( !node.queued )
    {
        node.queued = true;
        m_scheduled_nodes.push( nodeId, node.level );
    }
}

inline void react_graph::re_schedule_node( const node_id nodeId )
{
    node_data& node = m_node_data[nodeId];
    recalculate_successor_levels( node );
    m_scheduled_nodes.push( nodeId, node.level );
}

inline void react_graph::schedule_successors( node_data& parentNode )
{
    for( const node_id successorId : parentNode.successors )
        schedule_node( successorId );
}

inline void react_graph::propagate_node_change( const node_id nodeId )
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

inline void react_graph::finalize_changed_nodes()
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

inline void react_graph::unregister_queued_nodes()
{
    assert( !m_propagation_is_in_progress );

    for( const node_id nodeId : m_nodes_queued_for_unregister )
        unregister_node( nodeId );
    m_nodes_queued_for_unregister.clear();
}

UREACT_WARN_UNUSED_RESULT inline bool react_graph::topological_queue::fetch_next()
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

inline std::shared_ptr<react_graph> make_react_graph()
{
    return std::make_shared<react_graph>();
}

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_GRAPH_IMPL_INL
