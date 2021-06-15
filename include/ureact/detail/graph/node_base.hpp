#pragma once

#include "ureact/detail/reactive_node_interface.hpp"
#include "ureact/detail/context.hpp"

namespace ureact
{
namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// node_base
///////////////////////////////////////////////////////////////////////////////////////////////////
class node_base : public reactive_node
{
public:
    explicit node_base( context& context )
        : m_context( context )
    {}

    // Nodes can't be copied
    node_base( const node_base& ) = delete;
    node_base& operator=( const node_base& ) = delete;
    node_base( node_base&& ) noexcept = delete;
    node_base& operator=( node_base&& ) noexcept = delete;

    ~node_base() override = default;

    context& get_context() const
    {
        return m_context;
    }

    react_graph& get_graph()
    {
        return _get_internals( m_context ).get_graph();
    }

    const react_graph& get_graph() const
    {
        return _get_internals( m_context ).get_graph();
    }

private:
    context& m_context;
};

} // namespace detail
} // namespace ureact
