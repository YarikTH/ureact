//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_GRAPH_IMPL_HPP
#define UREACT_DETAIL_GRAPH_IMPL_HPP

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

class transaction;

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

private:
    friend class ureact::transaction;

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

    void recalculate_successor_levels( node_data& node );

    void schedule_node( node_id nodeId );

    void schedule_successors( node_data& node );

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
    node_id nodeId, const std::weak_ptr<reactive_node_interface>& nodePtr )
{
    assert( nodeId.context_id() == m_id );
    assert( nodePtr.use_count() > 0 );

    auto& node = m_node_data[nodeId];
    node.node_ptr = nodePtr;
}

inline void react_graph::unregister_node( node_id nodeId )
{
    assert( nodeId.context_id() == m_id );
    assert( m_node_data[nodeId].successors.empty() );

    if( can_unregister_node() )
        m_node_data.erase( nodeId );
    else
        m_nodes_queued_for_unregister.add( nodeId );
}

inline void react_graph::attach_node( node_id nodeId, node_id parentId )
{
    assert( nodeId.context_id() == m_id );
    assert( parentId.context_id() == m_id );

    auto& node = m_node_data[nodeId];
    auto& parent = m_node_data[parentId];

    parent.successors.add( nodeId );

    if( node.level <= parent.level )
    {
        node.level = parent.level + 1;
    }
}

inline void react_graph::detach_node( node_id nodeId, node_id parentId )
{
    assert( nodeId.context_id() == m_id );
    assert( parentId.context_id() == m_id );

    auto& parent = m_node_data[parentId];
    auto& successors = parent.successors;

    successors.remove( nodeId );
}

inline void react_graph::push_input( node_id nodeId )
{
    assert( !m_propagation_is_in_progress );

    schedule_node( nodeId );

    if( m_transaction_level == 0 )
    {
        propagate();
    }
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

    // Propagate changes
    while( m_scheduled_nodes.fetch_next() )
    {
        for( node_id nodeId : m_scheduled_nodes.next_values() )
        {
            auto& node = m_node_data[nodeId];
            if( auto nodePtr = node.node_ptr.lock() )
            {
                // A predecessor of this node has shifted to a lower level?
                if( node.level < node.new_level )
                {
                    // Re-schedule this node
                    node.level = node.new_level;

                    recalculate_successor_levels( node );
                    m_scheduled_nodes.push( nodeId, node.level );
                    continue;
                }

                const update_result result = nodePtr->update();

                // Topology changed?
                if( result == update_result::shifted )
                {
                    // Re-schedule this node
                    recalculate_successor_levels( node );
                    m_scheduled_nodes.push( nodeId, node.level );
                    continue;
                }

                if( result == update_result::changed )
                {
                    m_changed_nodes.add( nodeId );
                    schedule_successors( node );
                }
            }

            node.queued = false;
        }
    }

    // Cleanup buffers in changed nodes etc
    for( node_id nodeId : m_changed_nodes )
    {
        auto& node = m_node_data[nodeId];
        if( auto nodePtr = node.node_ptr.lock() )
        {
            nodePtr->finalize();
        }
    }
    m_changed_nodes.clear();

    m_propagation_is_in_progress = false;

    unregister_queued_nodes();
}

inline void react_graph::recalculate_successor_levels( node_data& node )
{
    for( node_id successorId : node.successors )
    {
        auto& successor = m_node_data[successorId];

        if( successor.new_level <= node.level )
        {
            successor.new_level = node.level + 1;
        }
    }
}

inline void react_graph::schedule_node( const node_id nodeId )
{
    auto& node = m_node_data[nodeId];

    if( !node.queued )
    {
        node.queued = true;
        m_scheduled_nodes.push( nodeId, node.level );
    }
}

inline void react_graph::schedule_successors( node_data& node )
{
    for( const node_id successorId : node.successors )
        schedule_node( successorId );
}

inline void react_graph::unregister_queued_nodes()
{
    assert( !m_propagation_is_in_progress );

    for( node_id nodeId : m_nodes_queued_for_unregister )
        unregister_node( nodeId );
    m_nodes_queued_for_unregister.clear();
}

UREACT_WARN_UNUSED_RESULT inline bool react_graph::topological_queue::fetch_next()
{
    // Throw away previous values
    m_next_data.clear();

    // Find min level of nodes in queue data
    auto minimal_level = std::numeric_limits<int>::max();
    for( const auto& e : m_queue_data )
    {
        if( minimal_level > e.second )
        {
            minimal_level = e.second;
        }
    }

    // Swap entries with min level to the end
    const auto p = detail::partition( m_queue_data.begin(),
        m_queue_data.end(),
        [minimal_level]( const entry& e ) { return e.second != minimal_level; } );

    // Reserve once to avoid multiple re-allocations
    const auto to_reserve = static_cast<size_t>( std::distance( p, m_queue_data.end() ) );
    m_next_data.reserve( to_reserve );

    // Move min level values to next data
    for( auto it = p, ite = m_queue_data.end(); it != ite; ++it )
    {
        m_next_data.push_back( it->first );
    }

    // Truncate moved entries
    const auto to_resize = static_cast<size_t>( std::distance( m_queue_data.begin(), p ) );
    m_queue_data.resize( to_resize );

    return !m_next_data.empty();
}

class context_internals
{
public:
    explicit context_internals(
        std::shared_ptr<detail::react_graph> graph = std::make_shared<react_graph>() )
        : m_graph_ptr( std::move( graph ) )
    {}

    UREACT_WARN_UNUSED_RESULT react_graph& get_graph()
    {
        return *m_graph_ptr;
    }

    UREACT_WARN_UNUSED_RESULT const react_graph& get_graph() const
    {
        return *m_graph_ptr;
    }

private:
    std::shared_ptr<react_graph> m_graph_ptr;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_GRAPH_IMPL_HPP
