#pragma once

#include <utility>
#include <type_traits>
#include <vector>

#include "ureact/detail/i_reactive_node.hpp"
#include "ureact/detail/turn_base.hpp"
#include "ureact/detail/util.hpp"

namespace ureact { namespace detail {

namespace toposort {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// topo_queue - Sequential
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T, typename level_func_t>
class topo_queue
{
private:
    struct entry;

public:
    // Store the level as part of the entry for cheap comparisons
    using queue_data_t    = std::vector<entry>;
    using next_data_t     = std::vector<T>;

    topo_queue() = default;
    
    template
    <
        typename in_f,
        class = typename std::enable_if<!is_same_decay<in_f,topo_queue>::value>::type
    >
    explicit topo_queue(in_f&& level_func) :
        m_level_func( std::forward<in_f>( level_func ) )
    {}

    void push(const T& value)
    {
        m_queue_data.emplace_back( value, m_level_func( value ) );
    }

    bool fetch_next()
    {
        // Throw away previous values
        m_next_data.clear();

        // Find min level of nodes in queue data
        m_min_level = (std::numeric_limits<int>::max)();
        for (const auto& e : m_queue_data)
            if ( m_min_level > e.m_level)
                m_min_level = e.m_level;

        // Swap entries with min level to the end
        auto p = std::partition(
            m_queue_data.begin(),
            m_queue_data.end(),
            level_comp_functor{ m_min_level } );

        // Reserve once to avoid multiple re-allocations
        const auto to_reserve = static_cast<size_t>(std::distance(p, m_queue_data.end() ));
        m_next_data.reserve( to_reserve );

        // Move min level values to next data
        for ( auto it = p; it != m_queue_data.end(); ++it)
            m_next_data.push_back( std::move( it->m_value ) );

        // Truncate moved entries
        const auto to_resize = static_cast<size_t>(std::distance( m_queue_data.begin(), p ));
        m_queue_data.resize( to_resize );

        return !m_next_data.empty();
    }

    const next_data_t& next_values() const  { return m_next_data; }

private:
    struct entry
    {
        entry() = default;
        
        entry(const T& value, int level) noexcept : m_value( value ), m_level( level ) {}

        T       m_value{};
        int     m_level{};
    };

    struct level_comp_functor
    {
        explicit level_comp_functor(int level) : m_level( level ) {}

        bool operator()(const entry& e) const { return e.m_level != m_level; }

        const int m_level;
    };

    next_data_t   m_next_data;
    queue_data_t  m_queue_data;

    level_func_t  m_level_func;

    int         m_min_level = (std::numeric_limits<int>::max)();
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// Functors
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct get_level_functor
{
    int operator()(const T* x) const { return x->level; }
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// i_reactive_engine
///////////////////////////////////////////////////////////////////////////////////////////////////
struct i_reactive_engine
{
    using node_t = reactive_node;
    using turn_t = turn_base;

    virtual void on_turn_admission_start(turn_t& /*turn*/) = 0;
    virtual void on_turn_admission_end(turn_t& /*turn*/) = 0;

    virtual void on_input_change(node_t& /*node*/, turn_t& /*turn*/) = 0;

    virtual void propagate(turn_t& /*turn*/) = 0;

    virtual void on_node_create(node_t& /*node*/) = 0;
    virtual void on_node_destroy(node_t& /*node*/) = 0;

    virtual void on_node_attach(node_t& /*node*/, node_t& /*parent*/) = 0;
    virtual void on_node_detach(node_t& /*node*/, node_t& /*parent*/) = 0;

    virtual void on_node_pulse(node_t& /*node*/, turn_t& /*turn*/) = 0;
    virtual void on_node_idle_pulse(node_t& /*node*/, turn_t& /*turn*/) = 0;

    virtual void on_dynamic_node_attach(node_t& /*node*/, node_t& /*parent*/, turn_t& /*turn*/) = 0;
    virtual void on_dynamic_node_detach(node_t& /*node*/, node_t& /*parent*/, turn_t& /*turn*/) = 0;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// engine_base
///////////////////////////////////////////////////////////////////////////////////////////////////
class engine_base : public i_reactive_engine
{
public:
    void on_node_attach(node_t& node, node_t& parent) override;
    void on_node_detach(node_t& node, node_t& parent) override;

    void on_input_change(node_t& node, turn_t& turn) override;
    void on_node_pulse(node_t& node, turn_t& turn) override;
protected:
    virtual void process_children(node_t& node, turn_t& turn) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// seq_engine_base
///////////////////////////////////////////////////////////////////////////////////////////////////
class seq_engine_base final : public engine_base
{
public:
    using topo_queue_t = topo_queue<reactive_node*, get_level_functor<reactive_node>>;

    void propagate(turn_base& turn) override;

    void on_dynamic_node_attach( reactive_node& node, reactive_node& parent, turn_base& turn ) override;
    void on_dynamic_node_detach( reactive_node& node, reactive_node& parent, turn_base& turn ) override;

    void on_turn_admission_start(turn_t& /*turn*/) override {}
    void on_turn_admission_end(turn_t& /*turn*/) override {}
    
    void on_node_create(node_t& /*node*/) override {}
    void on_node_destroy(node_t& /*node*/) override {}
    
    void on_node_idle_pulse(node_t& /*node*/, turn_t& /*turn*/) override {}
private:
    static void invalidate_successors( reactive_node& node );

    void process_children( reactive_node& node, turn_base& turn ) override;

    topo_queue_t    m_scheduled_nodes;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// engine_base
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void engine_base::on_node_attach(node_t& node, node_t& parent)
{
    parent.successors.add(node);

    if (node.level <= parent.level)
        node.level = parent.level + 1;
}

inline void engine_base::on_node_detach(node_t& node, node_t& parent)
{
    parent.successors.remove(node);
}

inline void engine_base::on_input_change(node_t& node, turn_t& turn)
{
    process_children(node, turn);
}

inline void engine_base::on_node_pulse(node_t& node, turn_t& turn)
{
    process_children(node, turn);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// seq_engine_base
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void seq_engine_base::propagate(turn_base& turn)
{
    while (m_scheduled_nodes.fetch_next())
    {
        for (auto* cur_node : m_scheduled_nodes.next_values())
        {
            if (cur_node->level < cur_node->new_level)
            {
                cur_node->level = cur_node->new_level;
                invalidate_successors(*cur_node);
                m_scheduled_nodes.push( cur_node );
                continue;
            }

            cur_node->queued = false;
            cur_node->tick(turn);
        }
    }
}

inline void seq_engine_base::on_dynamic_node_attach( reactive_node& node, reactive_node& parent, turn_base&  /*unused*/)
{
    this->on_node_attach(node, parent);
    
    invalidate_successors(node);

    // Re-schedule this node
    node.queued = true;
    m_scheduled_nodes.push( &node );
}

inline void seq_engine_base::on_dynamic_node_detach( reactive_node& node, reactive_node& parent, turn_base&  /*unused*/)
{
    this->on_node_detach(node, parent);
}

inline void seq_engine_base::process_children( reactive_node& node, turn_base&  /*turn*/)
{
    // add children to queue
    for (auto* succ : node.successors)
    {
        if (!succ->queued)
        {
            succ->queued = true;
            m_scheduled_nodes.push( succ );
        }
    }
}

inline void seq_engine_base::invalidate_successors( reactive_node& node )
{
    for (auto* succ : node.successors)
    {
        if (succ->new_level <= node.level)
            succ->new_level = node.level + 1;
    }
}

} // ~namespace toposort

using toposort_engine = ::ureact::detail::toposort::seq_engine_base;

}}
