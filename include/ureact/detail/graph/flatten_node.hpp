#pragma once

#include <utility>

#include "ureact/detail/graph/signal_node.hpp"

namespace ureact { namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// flatten_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename outer_t,
    typename inner_t
>
class flatten_node : public signal_node<inner_t>
{
public:
    flatten_node(context* context,
                std::shared_ptr<signal_node<outer_t>> outer,
                const std::shared_ptr<signal_node<inner_t>>& inner) :
        flatten_node::signal_node( context, inner->value_ref() ),
        outer_(std::move( outer )),
        inner_( inner )
    {
        flatten_node::get_context()->on_node_create(*this);
        flatten_node::get_context()->on_node_attach(*this, *outer_);
        flatten_node::get_context()->on_node_attach(*this, *inner_);
    }

    ~flatten_node() override
    {
        flatten_node::get_context()->on_node_detach(*this, *inner_);
        flatten_node::get_context()->on_node_detach(*this, *outer_);
        flatten_node::get_context()->on_node_destroy(*this);
    }
    
    // Nodes can't be copied
    flatten_node(const flatten_node&) = delete;
    flatten_node& operator=(const flatten_node&) = delete;
    flatten_node(flatten_node&&) noexcept = delete;
    flatten_node& operator=(flatten_node&&) noexcept = delete;
    
    void tick(turn_base& turn) override
    {
        const auto& new_inner = get_node_ptr(outer_->value_ref());

        if (new_inner != inner_)
        {
            // Topology has been changed
            auto old_inner = inner_;
            inner_ = new_inner;

            flatten_node::get_context()->on_dynamic_node_detach(*this, *old_inner, turn);
            flatten_node::get_context()->on_dynamic_node_attach(*this, *new_inner, turn);

            return;
        }
        
        if (! equals(this->value_, inner_->value_ref()))
        {
            this->value_ = inner_->value_ref();
            flatten_node::get_context()->on_node_pulse(*this, turn);
        }
        else
        {
            flatten_node::get_context()->on_node_idle_pulse(*this, turn);
        }
    }
    
private:
    std::shared_ptr<signal_node<outer_t>>   outer_;
    std::shared_ptr<signal_node<inner_t>>   inner_;
};

}}
