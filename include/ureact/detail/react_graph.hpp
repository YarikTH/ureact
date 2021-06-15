#pragma once

#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>

#include "ureact/detail/observer_base.hpp"

namespace ureact
{
namespace detail
{

struct input_node_interface;
class observer_interface;

class react_graph
{
public:
    react_graph() = default;

    template <typename F>
    void do_transaction( F&& func )
    {
        // Phase 1 - Input admission
        ++m_transaction_level;
        func();
        --m_transaction_level;

        if( m_transaction_level != 0 )
        {
            return;
        }

        // Phase 2 - apply_helper input node changes
        bool should_propagate = false;
        for( auto* p : m_changed_inputs )
        {
            if( p->apply_input() )
            {
                should_propagate = true;
            }
        }
        m_changed_inputs.clear();

        // Phase 3 - propagate changes
        if( should_propagate )
        {
            propagate();
        }

        detach_queued_observers();
    }

    template <typename R, typename V>
    void add_input( R& r, V&& v )
    {
        if( m_transaction_level > 0 )
        {
            add_transaction_input( r, std::forward<V>( v ) );
        }
        else
        {
            add_simple_input( r, std::forward<V>( v ) );
        }
    }

    template <typename R, typename F>
    void modify_input( R& r, const F& func )
    {
        if( m_transaction_level > 0 )
        {
            modify_transaction_input( r, func );
        }
        else
        {
            modify_simple_input( r, func );
        }
    }

    void queue_observer_for_detach( observer_interface& obs )
    {
        m_detached_observers.push_back( &obs );
    }

    void propagate();

    void on_node_attach( reactive_node& node, reactive_node& parent );
    void on_node_detach( reactive_node& node, reactive_node& parent );

    void on_input_change( reactive_node& node );
    void on_node_pulse( reactive_node& node );

    void on_dynamic_node_attach( reactive_node& node, reactive_node& parent );
    void on_dynamic_node_detach( reactive_node& node, reactive_node& parent );

private:
    class topological_queue
    {
    public:
        using value_type = reactive_node*;

        topological_queue() = default;
    
        void push( const value_type& value, const int level )
        {
            m_queue_data.emplace_back( value, level );
        }
    
        bool fetch_next();
    
        const std::vector<value_type>& next_values() const
        {
            return m_next_data;
        }
    
    private:
        using entry = std::pair<value_type, int>;
    
        std::vector<value_type> m_next_data;
        std::vector<entry> m_queue_data;
    };
    
    void detach_queued_observers()
    {
        for( auto* o : m_detached_observers )
        {
            o->unregister_self();
        }
        m_detached_observers.clear();
    }

    // Create a turn with a single input
    template <typename R, typename V>
    void add_simple_input( R& r, V&& v )
    {
        r.add_input( std::forward<V>( v ) );

        if( r.apply_input() )
        {
            propagate();
        }

        detach_queued_observers();
    }

    template <typename R, typename F>
    void modify_simple_input( R& r, const F& func )
    {
        r.modify_input( func );

        if( r.apply_input() )
        {
            propagate();
        }

        detach_queued_observers();
    }

    // This input is part of an active transaction
    template <typename R, typename V>
    void add_transaction_input( R& r, V&& v )
    {
        r.add_input( std::forward<V>( v ) );

        m_changed_inputs.push_back( &r );
    }

    template <typename R, typename F>
    void modify_transaction_input( R& r, const F& func )
    {
        r.modify_input( func );

        m_changed_inputs.push_back( &r );
    }
    
    static void invalidate_successors( reactive_node& node );

    void process_children( reactive_node& node );

    topological_queue m_scheduled_nodes;

    int m_transaction_level = 0;

    std::vector<input_node_interface*> m_changed_inputs;

    std::vector<observer_interface*> m_detached_observers;
};

inline bool react_graph::topological_queue::fetch_next()
{
    // Throw away previous values
    m_next_data.clear();

    // Find min level of nodes in queue data
    int minimal_level = std::numeric_limits<int>::max();
    for( const auto& e : m_queue_data )
    {
        if( minimal_level > e.second )
        {
            minimal_level = e.second;
        }
    }

    // Swap entries with min level to the end
    const auto p = std::partition( m_queue_data.begin(),
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

inline void react_graph::on_node_attach( reactive_node& node, reactive_node& parent )
{
    parent.successors.add( node );

    if( node.level <= parent.level )
    {
        node.level = parent.level + 1;
    }
}

inline void react_graph::on_node_detach( reactive_node& node, reactive_node& parent )
{
    parent.successors.remove( node );
}

inline void react_graph::on_input_change( reactive_node& node )
{
    process_children( node );
}

inline void react_graph::on_node_pulse( reactive_node& node )
{
    process_children( node );
}

inline void react_graph::propagate()
{
    while( m_scheduled_nodes.fetch_next() )
    {
        for( auto* cur_node : m_scheduled_nodes.next_values() )
        {
            if( cur_node->level < cur_node->new_level )
            {
                cur_node->level = cur_node->new_level;
                invalidate_successors( *cur_node );
                m_scheduled_nodes.push( cur_node, cur_node->level );
                continue;
            }

            cur_node->queued = false;
            cur_node->tick();
        }
    }
}

inline void react_graph::on_dynamic_node_attach( reactive_node& node, reactive_node& parent )
{
    on_node_attach( node, parent );

    invalidate_successors( node );

    // Re-schedule this node
    node.queued = true;
    m_scheduled_nodes.push( &node, node.level );
}

inline void react_graph::on_dynamic_node_detach( reactive_node& node, reactive_node& parent )
{
    on_node_detach( node, parent );
}

inline void react_graph::process_children( reactive_node& node )
{
    // add children to queue
    for( auto* succ : node.successors )
    {
        if( !succ->queued )
        {
            succ->queued = true;
            m_scheduled_nodes.push( succ, succ->level );
        }
    }
}

inline void react_graph::invalidate_successors( reactive_node& node )
{
    for( auto* succ : node.successors )
    {
        if( succ->new_level <= node.level )
        {
            succ->new_level = node.level + 1;
        }
    }
}

} // namespace detail
} // namespace ureact
