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
        m_op( std::forward<args_t>( args ) ... )
    {
        this->m_value = m_op.evaluate();

        signal_op_node::get_context()->on_node_create(*this);
        m_op.template attach( *this );
    }

    signal_op_node(const signal_op_node&) = delete;
    signal_op_node& operator=(const signal_op_node&) = delete;
    signal_op_node(signal_op_node&&) noexcept = delete;
    signal_op_node& operator=(signal_op_node&&) noexcept = delete;
    
    ~signal_op_node() override
    {
        if (!m_was_op_stolen)
            m_op.template detach( *this );
        signal_op_node::get_context()->on_node_destroy(*this);
    }

    void tick(turn_base& turn) override
    {
        bool changed = false;

        {// timer
            S new_value = m_op.evaluate();

            if (! equals( this->m_value, new_value ))
            {
                this->m_value = std::move( new_value );
                changed = true;
            }
        }// ~timer
        
        if (changed)
            signal_op_node::get_context()->on_node_pulse(*this, turn);
        else
            signal_op_node::get_context()->on_node_idle_pulse(*this, turn);
    }
    
    op_t steal_op()
    {
        assert( !m_was_op_stolen && "Op was already stolen." );
        m_was_op_stolen = true;
        m_op.template detach( *this );
        return std::move( m_op );
    }

private:
    op_t    m_op;
    bool    m_was_op_stolen = false;
};

}}
