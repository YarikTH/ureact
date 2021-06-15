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
    using engine_t = react_graph::engine_t;
    using node_t = react_graph::node_t;

    context_internals()
        : m_graph( new react_graph() )
    {}

    engine_t& get_engine()
    {
        return m_graph->get_engine();
    }

    void on_input_change( node_t& node )
    {
        get_engine().on_input_change( node );
    }

    void on_node_attach( node_t& node, node_t& parent )
    {
        get_engine().on_node_attach( node, parent );
    }

    void on_node_detach( node_t& node, node_t& parent )
    {
        get_engine().on_node_detach( node, parent );
    }

    void on_node_pulse( node_t& node )
    {
        get_engine().on_node_pulse( node );
    }

    void on_dynamic_node_attach( node_t& node, node_t& parent )
    {
        get_engine().on_dynamic_node_attach( node, parent );
    }

    void on_dynamic_node_detach( node_t& node, node_t& parent )
    {
        get_engine().on_dynamic_node_detach( node, parent );
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
