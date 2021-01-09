#pragma once

#include "ureact/detail/turn_base.hpp"

namespace ureact { namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// node_base
///////////////////////////////////////////////////////////////////////////////////////////////////
class node_base : public reactive_node
{
public:
    explicit node_base(context* context)
        : context_(context)
    {
    }

    // Nodes can't be copied
    node_base(const node_base&) = delete;
    node_base& operator=(const node_base&) = delete;
    node_base(node_base&&) noexcept = delete;
    node_base& operator=(node_base&&) noexcept = delete;
    
    ~node_base() override = default;
    
    context* get_context() const             { return context_; }
private:
    context* context_;
};

}}
