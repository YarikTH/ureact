#pragma once

#include "ureact/detail/turn_t.hpp"

namespace ureact { namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// node_base
///////////////////////////////////////////////////////////////////////////////////////////////////
class node_base : public reactive_node
{
public:
    explicit node_base(context* context)
        : m_context( context )
    {
    }

    // Nodes can't be copied
    node_base(const node_base&) = delete;
    node_base& operator=(const node_base&) = delete;
    node_base(node_base&&) noexcept = delete;
    node_base& operator=(node_base&&) noexcept = delete;
    
    ~node_base() override = default;
    
    context* get_context() const             { return m_context; }
private:
    context* m_context;
};

}}
