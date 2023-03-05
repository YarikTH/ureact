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

/// Utility class to defer self detach of observers
class deferred_observer_detacher
{
public:
    void queue_observer_for_detach( observer_interface& obs )
    {
        m_detached_observers.push_back( &obs );
    }

protected:
    void detach_queued_observers()
    {
        for( observer_interface* o : m_detached_observers )
        {
            o->unregister_self();
        }
        m_detached_observers.clear();
    }

    std::vector<observer_interface*> m_detached_observers;
};

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
    : public deferred_observer_detacher
#if !defined( NDEBUG )
    , public callback_sanitizer
#endif
{
public:
    react_graph() = default;

    node_id register_node( reactive_node_interface* nodePtr );
    void unregister_node( node_id nodeId );

    void attach_node( node_id nodeId, node_id parentId );
    void detach_node( node_id nodeId, node_id parentId );

    void push_input( node_id nodeId )
    {
        m_changed_inputs.add( nodeId );

        if( m_transaction_level == 0 )
        {
            propagate();
        }
    }

private:
    friend class ureact::transaction;

    void propagate()
    {
        // Fill update queue with successors of changed inputs
        for( node_id nodeId : m_changed_inputs )
        {
            auto& node = m_node_data[nodeId];
            auto* nodePtr = node.node_ptr;

            const update_result result = nodePtr->update();

            if( result == update_result::changed )
            {
                m_changed_nodes.push_back( nodePtr );
                schedule_successors( node );
            }
        }
        m_changed_inputs.clear();

        // Propagate changes
        while( m_scheduled_nodes.fetch_next() )
        {
            for( node_id nodeId : m_scheduled_nodes.next_values() )
            {
                auto& node = m_node_data[nodeId];
                auto* nodePtr = node.node_ptr;

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
                    m_changed_nodes.push_back( nodePtr );
                    schedule_successors( node );
                }

                node.queued = false;
            }
        }

        // Cleanup buffers in changed nodes etc
        for( reactive_node_interface* nodePtr : m_changed_nodes )
            nodePtr->finalize();
        m_changed_nodes.clear();

        detach_queued_observers();
    }

    struct node_data
    {
        UREACT_MAKE_NONCOPYABLE( node_data );
        UREACT_MAKE_MOVABLE( node_data );

        explicit node_data( reactive_node_interface* node_ptr )
            : node_ptr( node_ptr )
        {}

        int level = 0;
        int new_level = 0;
        bool queued = false;

        reactive_node_interface* node_ptr = nullptr;

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

    void recalculate_successor_levels( node_data& node );

    void schedule_successors( node_data& node );

    slot_map<node_data> m_node_data;

    topological_queue m_scheduled_nodes;

    int m_transaction_level = 0;

    node_id_vector m_changed_inputs;

    // local to propagate. Moved here to not reallocate
    std::vector<reactive_node_interface*> m_changed_nodes;
};

inline node_id react_graph::register_node( reactive_node_interface* nodePtr )
{
    return node_id{ m_node_data.insert( node_data{ nodePtr } ) };
}

inline void react_graph::unregister_node( node_id nodeId )
{
    assert( m_node_data[nodeId].successors.empty() );
    m_node_data.erase( nodeId );
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

inline void react_graph::attach_node( node_id nodeId, node_id parentId )
{
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
    auto& parent = m_node_data[parentId];
    auto& successors = parent.successors;

    successors.remove( nodeId );
}

inline void react_graph::schedule_successors( node_data& node )
{
    // add children to queue
    for( node_id successorId : node.successors )
    {
        auto& successor = m_node_data[successorId];

        if( !successor.queued )
        {
            successor.queued = true;
            m_scheduled_nodes.push( successorId, successor.level );
        }
    }
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

class context_internals
{
public:
    context_internals() = default;

    UREACT_WARN_UNUSED_RESULT react_graph& get_graph()
    {
        return *m_graph_ptr;
    }

    UREACT_WARN_UNUSED_RESULT const react_graph& get_graph() const
    {
        return *m_graph_ptr;
    }

private:
    std::shared_ptr<react_graph> m_graph_ptr = std::make_shared<react_graph>();
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_GRAPH_IMPL_HPP
