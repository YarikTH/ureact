#pragma once

#include "ureact/detail/toposort_engine.hpp"

#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <utility>
#include <type_traits>
#include <vector>

#include "observer_base.hpp"

namespace ureact { namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
struct i_input_node;
class i_observer;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// input_manager
///////////////////////////////////////////////////////////////////////////////////////////////////
class input_manager
{
public:
    using engine_t = toposort_engine;
    using node_t = engine_t::node_t;
    using turn_t = engine_t::turn_t;
    
    input_manager()
        : m_engine( new toposort_engine() )
    {
    }

    template <typename F>
    void do_transaction(F&& func)
    {
        const bool is_top_transaction = transactionLevel_ == 0;
        
        // Phase 1 - Input admission
        ++transactionLevel_;

        toposort::i_reactive_engine::turn_t turn;
        
        if( is_top_transaction )
        {
            turn = next_turn_id();
            get_engine().on_turn_admission_start( turn );
            func();
            get_engine().on_turn_admission_end( turn );
        }
        else
        {
            func();
        }
        
        --transactionLevel_;

        if( is_top_transaction )
        {
            // Phase 2 - apply_helper input node changes
            bool should_propagate = false;
            for (auto* p : changed_inputs_)
                if (p->apply_input(turn))
                    should_propagate = true;
            changed_inputs_.clear();
    
            // Phase 3 - propagate changes
            if (should_propagate)
                get_engine().propagate( turn );
        
            detach_queued_observers();
        }
    }

    template <typename R, typename V>
    void add_input(R& r, V&& v)
    {
        if (is_transaction_active())
        {
            add_transaction_input(r, std::forward<V>(v));
        }
        else
        {
            add_simple_input(r, std::forward<V>(v));
        }
    }

    template <typename R, typename F>
    void modify_input(R& r, const F& func)
    {
        if (is_transaction_active())
        {
            modify_transaction_input(r, func);
        }
        else
        {
            modify_simple_input(r, func);
        }
    }

    void queue_observer_for_detach(i_observer& obs)
    {
        detached_observers_.push_back(&obs);
    }
    
    engine_t& get_engine() const
    {
        return *m_engine;
    }
    
private:
    turn_id_t next_turn_id()
    {
        return next_turn_id_++;
    }

    bool is_transaction_active() const
    {
        return transactionLevel_ > 0;
    }
    
    void detach_queued_observers()
    {
        for (auto* o : detached_observers_)
            o->unregister_self();
        detached_observers_.clear();
    }

    // Create a turn with a single input
    template <typename R, typename V>
    void add_simple_input(R& r, V&& v)
    {
        toposort::i_reactive_engine::turn_t turn( next_turn_id() );
        get_engine().on_turn_admission_start( turn );
        r.add_input(std::forward<V>(v));
        get_engine().on_turn_admission_end( turn );

        if (r.apply_input(turn))
            get_engine().propagate( turn );
    
        detach_queued_observers();
    }

    template <typename R, typename F>
    void modify_simple_input(R& r, const F& func)
    {
        toposort::i_reactive_engine::turn_t turn( next_turn_id() );
        get_engine().on_turn_admission_start( turn );
        r.modify_input(func);
        get_engine().on_turn_admission_end( turn );

        // Return value, will always be true
        r.apply_input(turn);

        get_engine().propagate( turn );
    
        detach_queued_observers();
    }

    // This input is part of an active transaction
    template <typename R, typename V>
    void add_transaction_input(R& r, V&& v)
    {
        r.add_input(std::forward<V>(v));
        changed_inputs_.push_back(&r);
    }

    template <typename R, typename F>
    void modify_transaction_input(R& r, const F& func)
    {
        r.modify_input(func);
        changed_inputs_.push_back(&r);
    }
    
    std::unique_ptr<engine_t> m_engine;
    
    turn_id_t next_turn_id_{ 0 };

    int  transactionLevel_ = 0;
    
    std::vector<i_input_node*>    changed_inputs_;
    
    std::vector<i_observer*>    detached_observers_;
};

}}
