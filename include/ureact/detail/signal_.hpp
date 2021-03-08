#pragma once

#include "ureact/detail/signal_base.hpp"

namespace ureact
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class signal : public ::ureact::detail::signal_base<S>
{
private:
    using node_t = ::ureact::detail::signal_node<S>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
    using value_t = S;

    // Default ctor
    signal() = default;

    // Constructor
    explicit signal( context* context )
        : signal::signal_base( context )
    {}

    // Copy ctor
    signal( const signal& ) = default;

    // Move ctor
    signal( signal&& other ) noexcept = default;

    // Node ctor
    explicit signal( context* context, node_ptr_t&& node_ptr )
        : signal::signal_base( context, std::move( node_ptr ) )
    {}

    // Copy assignment
    signal& operator=( const signal& ) = default;

    // Move assignment
    signal& operator=( signal&& other ) noexcept = default;

    ~signal() = default;

    const S& value() const
    {
        return signal::signal_base::get_value();
    }

    bool equals( const signal& other ) const
    {
        return signal::signal_base::equals( other );
    }

    bool is_valid() const
    {
        return signal::signal_base::is_valid();
    }

    S flatten() const
    {
        static_assert( is_signal<S>::value, "flatten requires a signal value type." );
        return ::ureact::flatten( *this );
    }
};

// Specialize for references
template <typename S>
class signal<S&> : public ::ureact::detail::signal_base<std::reference_wrapper<S>>
{
private:
    using node_t = ::ureact::detail::signal_node<std::reference_wrapper<S>>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
    using value_t = S;

    // Default ctor
    signal() = default;

    // Copy ctor
    signal( const signal& ) = default;

    // Move ctor
    signal( signal&& other ) noexcept
        : signal::signal_base( std::move( other ) )
    {}

    // Node ctor
    explicit signal( node_ptr_t&& node_ptr )
        : signal::signal_base( std::move( node_ptr ) )
    {}

    // Copy assignment
    signal& operator=( const signal& ) = default;

    // Move assignment
    signal& operator=( signal&& other ) noexcept
    {
        signal::signal_base::operator=( std::move( other ) );
        return *this;
    }

    ~signal() = default;

    const S& value() const
    {
        return signal::signal_base::get_value();
    }

    bool equals( const signal& other ) const
    {
        return signal::signal_base::equals( other );
    }

    bool is_valid() const
    {
        return signal::signal_base::is_valid();
    }
};

} // namespace ureact
