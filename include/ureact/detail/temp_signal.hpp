#pragma once

#include "ureact/detail/graph/signal_op_node.hpp"

namespace ureact
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// temp_signal
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S, typename op_t>
class temp_signal : public signal<S>
{
private:
    using node_t = ::ureact::detail::signal_op_node<S, op_t>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
    // Default ctor
    temp_signal() = default;

    // Constructor
    explicit temp_signal( context* context )
        : temp_signal::signal( context )
    {}

    // Copy ctor
    temp_signal( const temp_signal& ) = default;

    // Move ctor
    temp_signal( temp_signal&& other ) noexcept
        : temp_signal::signal( std::move( other ) )
    {}

    // Node ctor
    explicit temp_signal( context* context, node_ptr_t&& ptr )
        : temp_signal::signal( context, std::move( ptr ) )
    {}

    // Copy assignment
    temp_signal& operator=( const temp_signal& ) = default;

    // Move assignemnt
    temp_signal& operator=( temp_signal&& other ) noexcept
    {
        temp_signal::signal::operator=( std::move( other ) );
        return *this;
    }

    ~temp_signal() = default;

    op_t steal_op()
    {
        return std::move( static_cast<node_t*>( this->m_ptr.get() )->steal_op() );
    }
};

} // namespace ureact
