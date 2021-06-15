#pragma once

#include <memory>

#include "ureact/detail/reactive_input.hpp"

namespace ureact
{
namespace detail
{

class context_internals
{
public:
    using input_manager_t = ::ureact::detail::input_manager;
    using engine_t = input_manager_t::engine_t;
    using node_t = input_manager_t::node_t;

    context_internals()
        : m_input_manager( new input_manager_t() )
    {}

    engine_t& get_engine()
    {
        return m_input_manager->get_engine();
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
        m_input_manager->queue_observer_for_detach( obs );
    }

    template <typename R, typename V>
    void add_input( R& r, V&& v )
    {
        m_input_manager->add_input( r, std::forward<V>( v ) );
    }

    template <typename R, typename F>
    void modify_input( R& r, const F& func )
    {
        m_input_manager->modify_input( r, func );
    }

protected:
    std::unique_ptr<input_manager_t> m_input_manager;
};

} // namespace detail
} // namespace ureact
