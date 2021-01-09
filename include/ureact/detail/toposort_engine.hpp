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
        level_func_( std::forward<in_f>(level_func) )
    {}

    void push(const T& value)
    {
        queue_data_.emplace_back(value, level_func_(value));
    }

    bool fetch_next()
    {
        // Throw away previous values
        next_data_.clear();

        // Find min level of nodes in queue data
        min_level_ = (std::numeric_limits<int>::max)();
        for (const auto& e : queue_data_)
            if (min_level_ > e.level_)
                min_level_ = e.level_;

        // Swap entries with min level to the end
        auto p = std::partition(
            queue_data_.begin(),
            queue_data_.end(),
            level_comp_functor{ min_level_ });

        // Reserve once to avoid multiple re-allocations
        const auto to_reserve = static_cast<size_t>(std::distance(p, queue_data_.end()));
        next_data_.reserve(to_reserve);

        // Move min level values to next data
        for (auto it = p; it != queue_data_.end(); ++it)
            next_data_.push_back(std::move(it->value_));

        // Truncate moved entries
        const auto to_resize = static_cast<size_t>(std::distance(queue_data_.begin(), p));
        queue_data_.resize(to_resize);

        return !next_data_.empty();
    }

    const next_data_t& next_values() const  { return next_data_; }

private:
    struct entry
    {
        entry() = default;
        
        entry(const T& value, int level) noexcept : value_( value ), level_( level ) {}

        T       value_{};
        int     level_{};
    };

    struct level_comp_functor
    {
        explicit level_comp_functor(int level) : level_( level ) {}

        bool operator()(const entry& e) const { return e.level_ != level_; }

        const int level_;
    };

    next_data_t   next_data_;
    queue_data_t  queue_data_;

    level_func_t  level_func_;

    int         min_level_ = (std::numeric_limits<int>::max)();
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

    virtual void on_turn_admission_start(turn_t& /*turn*/){}
    virtual void on_turn_admission_end(turn_t& /*turn*/){}

    virtual void on_input_change(node_t& /*node*/, turn_t& /*turn*/){}

    virtual void propagate(turn_t& /*turn*/){}

    virtual void on_node_create(node_t& /*node*/){}
    virtual void on_node_destroy(node_t& /*node*/){}

    virtual void on_node_attach(node_t& /*node*/, node_t& /*parent*/){}
    virtual void on_node_detach(node_t& /*node*/, node_t& /*parent*/){}

    virtual void on_node_pulse(node_t& /*node*/, turn_t& /*turn*/){}
    virtual void on_node_idle_pulse(node_t& /*node*/, turn_t& /*turn*/){}

    virtual void on_dynamic_node_attach(node_t& /*node*/, node_t& /*parent*/, turn_t& /*turn*/){}
    virtual void on_dynamic_node_detach(node_t& /*node*/, node_t& /*parent*/, turn_t& /*turn*/){}
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

private:
    static void invalidate_successors( reactive_node& node );

    void process_children( reactive_node& node, turn_base& turn ) override;

    topo_queue_t    scheduled_nodes_;
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
    while (scheduled_nodes_.fetch_next())
    {
        for (auto* cur_node : scheduled_nodes_.next_values())
        {
            if (cur_node->level < cur_node->new_level)
            {
                cur_node->level = cur_node->new_level;
                invalidate_successors(*cur_node);
                scheduled_nodes_.push(cur_node);
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
    scheduled_nodes_.push(&node);
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
            scheduled_nodes_.push(succ);
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
