#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <type_traits>
#include <utility>
#include <vector>

#include "ureact/detail/toposort_engine.hpp"
#include "ureact/detail/observer_base.hpp"

namespace ureact
{
namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
struct input_node_interface;
class observer_interface;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// input_manager
///////////////////////////////////////////////////////////////////////////////////////////////////
class input_manager
{
public:
    using engine_t = toposort_engine;
    using node_t = engine_t::node_t;

    input_manager()
        : m_engine( new toposort_engine() )
    {}

    template <typename F>
    void do_transaction( F&& func )
    {
        const bool is_top_transaction = m_transaction_level == 0;

        // Phase 1 - Input admission
        ++m_transaction_level;

        turn_t current_turn;

        if ( is_top_transaction )
        {
            current_turn = next_turn_id();
            get_engine().on_turn_admission_start( current_turn );
            func();
            get_engine().on_turn_admission_end( current_turn );
        }
        else
        {
            func();
        }

        --m_transaction_level;

        if ( is_top_transaction )
        {
            // Phase 2 - apply_helper input node changes
            bool should_propagate = false;
            for ( auto* p : m_changed_inputs )
                if ( p->apply_input( current_turn ) )
                    should_propagate = true;
            m_changed_inputs.clear();

            // Phase 3 - propagate changes
            if ( should_propagate )
                get_engine().propagate( current_turn );

            detach_queued_observers();
        }
    }

    template <typename R, typename V>
    void add_input( R& r, V&& v )
    {
        if ( is_transaction_active() )
        {
            add_transaction_input( r, std::forward<V>( v ) );
        }
        else
        {
            add_simple_input( r, std::forward<V>( v ) );
        }
    }

    template <typename R, typename F>
    void modify_input( R& r, const F& func )
    {
        if ( is_transaction_active() )
        {
            modify_transaction_input( r, func );
        }
        else
        {
            modify_simple_input( r, func );
        }
    }

    void queue_observer_for_detach( observer_interface& obs )
    {
        m_detached_observers.push_back( &obs );
    }

    engine_t& get_engine() const
    {
        return *m_engine;
    }

private:
    turn_id_t next_turn_id()
    {
        return m_next_turn_id++;
    }

    bool is_transaction_active() const
    {
        return m_transaction_level > 0;
    }

    void detach_queued_observers()
    {
        for ( auto* o : m_detached_observers )
            o->unregister_self();
        m_detached_observers.clear();
    }

    // Create a turn with a single input
    template <typename R, typename V>
    void add_simple_input( R& r, V&& v )
    {
        turn_t turn( next_turn_id() );
        get_engine().on_turn_admission_start( turn );
        r.add_input( std::forward<V>( v ) );
        get_engine().on_turn_admission_end( turn );

        if ( r.apply_input( turn ) )
            get_engine().propagate( turn );

        detach_queued_observers();
    }

    template <typename R, typename F>
    void modify_simple_input( R& r, const F& func )
    {
        turn_t turn( next_turn_id() );
        get_engine().on_turn_admission_start( turn );
        r.modify_input( func );
        get_engine().on_turn_admission_end( turn );

        // Return value, will always be true
        r.apply_input( turn );

        get_engine().propagate( turn );

        detach_queued_observers();
    }

    // This input is part of an active transaction
    template <typename R, typename V>
    void add_transaction_input( R& r, V&& v )
    {
        r.add_input( std::forward<V>( v ) );
        m_changed_inputs.push_back( &r );
    }

    template <typename R, typename F>
    void modify_transaction_input( R& r, const F& func )
    {
        r.modify_input( func );
        m_changed_inputs.push_back( &r );
    }

    std::unique_ptr<engine_t> m_engine;

    turn_id_t m_next_turn_id{ 0 };

    int m_transaction_level = 0;

    std::vector<input_node_interface*> m_changed_inputs;

    std::vector<observer_interface*> m_detached_observers;
};

} // namespace detail
} // namespace ureact
