#pragma once

#include <cassert>
#include <iostream> // @todo delete it later

#include "ureact/detail/graph/signal_node.hpp"

namespace ureact
{
namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// const_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class const_node : public signal_node<S>
{
public:
    template <typename T>
    explicit const_node( context& context, T&& value )
        : const_node::signal_node( context, std::forward<T>( value ) )
    {
        const_node::signal_node::get_context().on_node_create( *this );
        std::cout << "const_node()\n";
    }

    ~const_node() override
    {
        const_node::signal_node::get_context().on_node_destroy( *this );
        std::cout << "~const_node()\n";
    }

    // LCOV_EXCL_START
    void tick( turn_t& /*turn*/ ) override
    {
        assert( false && "Ticked const_node" );
    }
    // LCOV_EXCL_STOP
};

} // namespace detail
} // namespace ureact
