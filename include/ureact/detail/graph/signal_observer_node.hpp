#pragma once

#include "ureact/detail/graph/observer_node.hpp"
#include "ureact/detail/graph/signal_node.hpp"

namespace ureact { namespace detail {

/// Observer functions can return values of this type to control further processing.
enum class observer_action
{
    next,           ///< Need to continue observing
    stop_and_detach ///< Need to stop observing
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_observer_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename S,
    typename func_t
>
class signal_observer_node :
    public observer_node
{
public:
    template <typename F>
    signal_observer_node(context* context, const std::shared_ptr<signal_node<S>>& subject, F&& func) :
        signal_observer_node::observer_node( context ),
        subject_( subject ),
        func_( std::forward<F>(func) )
    {
        get_context()->on_node_create(*this);
        get_context()->on_node_attach(*this, *subject);
    }

    signal_observer_node(const signal_observer_node&) = delete;
    signal_observer_node& operator=(const signal_observer_node&) = delete;
    signal_observer_node(signal_observer_node&&) noexcept = delete;
    signal_observer_node& operator=(signal_observer_node&&) noexcept = delete;
    
    ~signal_observer_node() override
    {
        get_context()->on_node_destroy(*this);
    }

    const char* get_node_type() const override        { return "signal_observer_node"; }

    void tick(turn_base& /*turn*/) override
    {
        bool should_detach = false;

        if (auto p = subject_.lock())
        {// timer
            if (func_(p->value_ref()) == observer_action::stop_and_detach)
                should_detach = true;
        }// ~timer

        if (should_detach)
            get_context()->get_input_manager().queue_observer_for_detach(*this);
    }

    void unregister_self() override
    {
        if (auto p = subject_.lock())
            p->unregister_observer(this);
    }

private:
    void detach_observer() override
    {
        if (auto p = subject_.lock())
        {
            get_context()->on_node_detach(*this, *p);
            subject_.reset();
        }
    }

    std::weak_ptr<signal_node<S>>  subject_;
    func_t                         func_;
};

}}
