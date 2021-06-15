#pragma once

#include <memory>

#include "ureact/detail/react_graph.hpp"

namespace ureact
{
namespace detail
{

class context_internals
{
public:
    context_internals()
        : m_graph( new react_graph() )
    {}

    void on_input_change( reactive_node& node )
    {
        m_graph->on_input_change( node );
    }

    void on_node_attach( reactive_node& node, reactive_node& parent )
    {
        m_graph->on_node_attach( node, parent );
    }

    void on_node_detach( reactive_node& node, reactive_node& parent )
    {
        m_graph->on_node_detach( node, parent );
    }

    void on_node_pulse( reactive_node& node )
    {
        m_graph->on_node_pulse( node );
    }

    void on_dynamic_node_attach( reactive_node& node, reactive_node& parent )
    {
        m_graph->on_dynamic_node_attach( node, parent );
    }

    void on_dynamic_node_detach( reactive_node& node, reactive_node& parent )
    {
        m_graph->on_dynamic_node_detach( node, parent );
    }

    void queue_observer_for_detach( observer_interface& obs )
    {
        m_graph->queue_observer_for_detach( obs );
    }

    template <typename R, typename V>
    void add_input( R& r, V&& v )
    {
        m_graph->add_input( r, std::forward<V>( v ) );
    }

    template <typename R, typename F>
    void modify_input( R& r, const F& func )
    {
        m_graph->modify_input( r, func );
    }

protected:
    std::unique_ptr<react_graph> m_graph;
};

} // namespace detail
} // namespace ureact
