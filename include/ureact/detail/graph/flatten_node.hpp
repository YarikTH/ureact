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
        m_outer(std::move( outer )),
        m_inner( inner )
    {
        flatten_node::get_context()->on_node_create(*this);
        flatten_node::get_context()->on_node_attach(*this, *m_outer);
        flatten_node::get_context()->on_node_attach(*this, *m_inner);
    }

    ~flatten_node() override
    {
        flatten_node::get_context()->on_node_detach(*this, *m_inner);
        flatten_node::get_context()->on_node_detach(*this, *m_outer);
        flatten_node::get_context()->on_node_destroy(*this);
    }
    
    // Nodes can't be copied
    flatten_node(const flatten_node&) = delete;
    flatten_node& operator=(const flatten_node&) = delete;
    flatten_node(flatten_node&&) noexcept = delete;
    flatten_node& operator=(flatten_node&&) noexcept = delete;
    
    void tick( turn_t& turn ) override
    {
        const auto& new_inner = get_node_ptr(m_outer->value_ref());

        if (new_inner != m_inner)
        {
            // Topology has been changed
            auto old_inner = m_inner;
            m_inner = new_inner;

            flatten_node::get_context()->on_dynamic_node_detach(*this, *old_inner, turn);
            flatten_node::get_context()->on_dynamic_node_attach(*this, *new_inner, turn);

            return;
        }
        
        if (! equals( this->m_value, m_inner->value_ref() ))
        {
            this->m_value = m_inner->value_ref();
            flatten_node::get_context()->on_node_pulse(*this, turn);
        }
        else
        {
            flatten_node::get_context()->on_node_idle_pulse(*this, turn);
        }
    }
    
private:
    std::shared_ptr<signal_node<outer_t>>   m_outer;
    std::shared_ptr<signal_node<inner_t>>   m_inner;
};

}}
