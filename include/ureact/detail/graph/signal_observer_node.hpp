#pragma once

#include "ureact/detail/graph/observer_node.hpp"
#include "ureact/detail/graph/signal_node.hpp"

namespace ureact
{
namespace detail
{

/// Observer functions can return values of this type to control further processing.
enum class observer_action
{
    next,           ///< Need to continue observing
    stop_and_detach ///< Need to stop observing
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_observer_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S, typename func_t>
class signal_observer_node : public observer_node
{
public:
    template <typename F>
    signal_observer_node(
        context& context, const std::shared_ptr<signal_node<S>>& subject, F&& func )
        : signal_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<F>( func ) )
    {
        get_graph().on_node_attach( *this, *subject );
    }

    signal_observer_node( const signal_observer_node& ) = delete;
    signal_observer_node& operator=( const signal_observer_node& ) = delete;
    signal_observer_node( signal_observer_node&& ) noexcept = delete;
    signal_observer_node& operator=( signal_observer_node&& ) noexcept = delete;

    void tick() override
    {
        bool should_detach = false;

        if( auto p = m_subject.lock() )
        {
            if( m_func( p->value_ref() ) == observer_action::stop_and_detach )
            {
                should_detach = true;
            }
        }

        if( should_detach )
        {
            get_graph().queue_observer_for_detach( *this );
        }
    }

    void unregister_self() override
    {
        if( auto p = m_subject.lock() )
        {
            p->unregister_observer( this );
        }
    }

private:
    void detach_observer() override
    {
        if( auto p = m_subject.lock() )
        {
            get_graph().on_node_detach( *this, *p );
            m_subject.reset();
        }
    }

    std::weak_ptr<signal_node<S>> m_subject;
    func_t m_func;
};

} // namespace detail
} // namespace ureact
