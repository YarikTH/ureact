#pragma once

#include "ureact/detail/reactive_input.hpp"

#include <memory>

namespace ureact {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// context
///////////////////////////////////////////////////////////////////////////////////////////////////
class context
{
public:
    using input_manager_t = ::ureact::detail::input_manager;
    using engine_t = input_manager_t::engine_t;
    using node_t = input_manager_t::node_t;
    using turn_t = ::ureact::detail::turn_t;

    context()
        : m_input_manager( new input_manager_t() )
    {
    }

    engine_t& get_engine()
    {
        return m_input_manager->get_engine();
    }

    input_manager_t& get_input_manager()
    {
        return *m_input_manager;
    }
    
    template <typename F>
    void do_transaction(F&& func)
    {
        get_input_manager().do_transaction( std::forward<F>( func ) );
    }
    
    void on_turn_admission_start( turn_t& turn )
    {
        get_engine().on_turn_admission_start(turn);
    }

    void on_turn_admission_end( turn_t& turn )
    {
        get_engine().on_turn_admission_end(turn);
    }

    void on_input_change(node_t& node, turn_t& turn )
    {
        get_engine().on_input_change(node, turn);
    }

    void propagate( turn_t& turn )
    {
        get_engine().propagate(turn);
    }

    void on_node_create(node_t& node)
    {
        get_engine().on_node_create(node);
    }

    void on_node_destroy(node_t& node)
    {
        get_engine().on_node_destroy(node);
    }

    void on_node_attach(node_t& node, node_t& parent)
    {
        get_engine().on_node_attach(node, parent);
    }

    void on_node_detach(node_t& node, node_t& parent)
    {
        get_engine().on_node_detach(node, parent);
    }

    void on_node_pulse(node_t& node, turn_t& turn )
    {
        get_engine().on_node_pulse(node, turn);
    }

    void on_node_idle_pulse(node_t& node, turn_t& turn )
    {
        get_engine().on_node_idle_pulse(node, turn);
    }

    void on_dynamic_node_attach(node_t& node, node_t& parent, turn_t& turn )
    {
        get_engine().on_dynamic_node_attach(node, parent, turn);
    }

    void on_dynamic_node_detach(node_t& node, node_t& parent, turn_t& turn )
    {
        get_engine().on_dynamic_node_detach(node, parent, turn);
    }
private:
    std::unique_ptr<input_manager_t> m_input_manager;
};

}
