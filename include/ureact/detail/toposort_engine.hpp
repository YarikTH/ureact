#pragma once

#include <type_traits>
#include <utility>
#include <vector>
#include <algorithm>

#include "ureact/detail/reactive_node_interface.hpp"
#include "ureact/detail/util.hpp"

namespace ureact
{
namespace detail
{

namespace toposort
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// topo_queue - Sequential
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, typename level_func_t>
class topo_queue
{
private:
    struct entry;

public:
    topo_queue() = default;

    void push( const T& value )
    {
        m_queue_data.emplace_back( value, level_func_t()( value ) );
    }

    [[nodiscard]] bool fetch_next()
    {
        // Throw away previous values
        m_next_data.clear();

        // Find min level of nodes in queue data
        int minimal_level = std::numeric_limits<int>::max();
        for( const auto& e : m_queue_data )
        {
            if( minimal_level > e.level )
            {
                minimal_level = e.level;
            }
        }

        // Swap entries with min level to the end
        const auto p = std::partition(
            m_queue_data.begin(), m_queue_data.end(), level_comp_functor{ minimal_level } );

        // Reserve once to avoid multiple re-allocations
        const auto to_reserve = static_cast<size_t>( std::distance( p, m_queue_data.end() ) );
        m_next_data.reserve( to_reserve );

        // Move min level values to next data
        for( auto it = p, ite = m_queue_data.end(); it != ite; ++it )
        {
            m_next_data.push_back( std::move( it->value ) );
        }

        // Truncate moved entries
        const auto to_resize = static_cast<size_t>( std::distance( m_queue_data.begin(), p ) );
        m_queue_data.resize( to_resize );

        return !m_next_data.empty();
    }

    [[nodiscard]] const std::vector<T>& next_values() const
    {
        return m_next_data;
    }

private:
    // Store the level as part of the entry for cheap comparisons
    struct entry
    {
        entry() = default;

        entry( const T& value, int level ) noexcept
            : value( value )
            , level( level )
        {}

        T value{};
        int level{};

        friend bool operator<( const entry& lhs, const entry& rhs )
        {
            return lhs.level < rhs.level;
        }
    };

    struct level_comp_functor
    {
        explicit level_comp_functor( int level )
            : level( level )
        {}

        bool operator()( const entry& e ) const
        {
            return e.level != level;
        }

        const int level;
    };

    std::vector<T> m_next_data;
    std::vector<entry> m_queue_data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Functors
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct get_level_functor
{
    int operator()( const T* x ) const
    {
        return x->level;
    }
};

class toposort_engine
{
public:
    using node_t = reactive_node;
    using topo_queue_t = topo_queue<node_t*, get_level_functor<node_t>>;

    void propagate();

    void on_node_attach( node_t& node, node_t& parent );
    void on_node_detach( node_t& node, node_t& parent );

    void on_input_change( node_t& node );
    void on_node_pulse( node_t& node );

    void on_dynamic_node_attach( node_t& node, node_t& parent );
    void on_dynamic_node_detach( node_t& node, node_t& parent );

private:
    static void invalidate_successors( node_t& node );

    void process_children( node_t& node );

    topo_queue_t m_scheduled_nodes;
};

inline void toposort_engine::on_node_attach( node_t& node, node_t& parent )
{
    parent.successors.add( node );

    if( node.level <= parent.level )
        node.level = parent.level + 1;
}

inline void toposort_engine::on_node_detach( node_t& node, node_t& parent )
{
    parent.successors.remove( node );
}

inline void toposort_engine::on_input_change( node_t& node )
{
    process_children( node );
}

inline void toposort_engine::on_node_pulse( node_t& node )
{
    process_children( node );
}

inline void toposort_engine::propagate()
{
    while( m_scheduled_nodes.fetch_next() )
    {
        for( auto* cur_node : m_scheduled_nodes.next_values() )
        {
            if( cur_node->level < cur_node->new_level )
            {
                cur_node->level = cur_node->new_level;
                invalidate_successors( *cur_node );
                m_scheduled_nodes.push( cur_node );
                continue;
            }

            cur_node->queued = false;
            cur_node->tick();
        }
    }
}

inline void toposort_engine::on_dynamic_node_attach( reactive_node& node, reactive_node& parent )
{
    this->on_node_attach( node, parent );

    invalidate_successors( node );

    // Re-schedule this node
    node.queued = true;
    m_scheduled_nodes.push( &node );
}

inline void toposort_engine::on_dynamic_node_detach( reactive_node& node, reactive_node& parent )
{
    this->on_node_detach( node, parent );
}

inline void toposort_engine::process_children( reactive_node& node )
{
    // add children to queue
    for( auto* succ : node.successors )
    {
        if( !succ->queued )
        {
            succ->queued = true;
            m_scheduled_nodes.push( succ );
        }
    }
}

inline void toposort_engine::invalidate_successors( reactive_node& node )
{
    for( auto* succ : node.successors )
    {
        if( succ->new_level <= node.level )
            succ->new_level = node.level + 1;
    }
}

} // namespace toposort

using toposort_engine = ::ureact::detail::toposort::toposort_engine;

} // namespace detail
} // namespace ureact
