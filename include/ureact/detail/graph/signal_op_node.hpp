#pragma once

#include "ureact/detail/graph/signal_node.hpp"

namespace ureact { namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_op_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename S,
    typename op_t
>
class signal_op_node :
    public signal_node<S>
{
public:
    template <typename ... args_t>
    explicit signal_op_node(context* context, args_t&& ... args) :
        signal_op_node::signal_node( context ),
        op_( std::forward<args_t>(args) ... )
    {
        this->value_ = op_.evaluate();

        signal_op_node::get_context()->on_node_create(*this);
        op_.template attach(*this);
    }

    signal_op_node(const signal_op_node&) = delete;
    signal_op_node& operator=(const signal_op_node&) = delete;
    signal_op_node(signal_op_node&&) noexcept = delete;
    signal_op_node& operator=(signal_op_node&&) noexcept = delete;
    
    ~signal_op_node() override
    {
        if (!was_op_stolen_)
            op_.template detach(*this);
        signal_op_node::get_context()->on_node_destroy(*this);
    }

    void tick(turn_base& turn) override
    {
        bool changed = false;

        {// timer
            S new_value = op_.evaluate();

            if (! equals(this->value_, new_value))
            {
                this->value_ = std::move(new_value);
                changed = true;
            }
        }// ~timer
        
        if (changed)
            signal_op_node::get_context()->on_node_pulse(*this, turn);
        else
            signal_op_node::get_context()->on_node_idle_pulse(*this, turn);
    }

    const char* get_node_type() const override        { return "signal_op_node"; }

    op_t steal_op()
    {
        assert(!was_op_stolen_ && "Op was already stolen.");
        was_op_stolen_ = true;
        op_.template detach(*this);
        return std::move(op_);
    }

private:
    op_t     op_;
    bool    was_op_stolen_ = false;
};

}}
