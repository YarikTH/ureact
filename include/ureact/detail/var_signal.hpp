#pragma once

#include "ureact/detail/graph/var_node.hpp"

namespace ureact
{
namespace detail
{
///////////////////////////////////////////////////////////////////////////////////////////////////
/// var_signal_base
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class var_signal_base : public signal<S>
{
public:
    // Default ctor
    var_signal_base() = default;

    template <typename T>
    explicit var_signal_base( T&& t )
        : var_signal_base::signal( std::forward<T>( t ) )
    {}

private:
    auto get_var_node() const -> var_node<S>*
    {
        return static_cast<var_node<S>*>(this->m_ptr.get());
    }
    
protected:
    
    template <typename T>
    void set_value( T&& new_value ) const
    {
        get_var_node()->template request_add_input(std::forward<T>(new_value));
    }

    template <typename F>
    void modify_value( const F& func ) const
    {
        get_var_node()->template request_modify_input(func);
    }
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// var_signal
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class var_signal : public detail::var_signal_base<S>
{
private:
    using node_t = ::ureact::detail::var_node<S>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
    // Default ctor
    var_signal() = default;

    // Copy ctor
    var_signal( const var_signal& ) = default;

    // Move ctor
    var_signal( var_signal&& other ) noexcept
        : var_signal::var_signal_base( std::move( other ) )
    {}

    // Node ctor
    explicit var_signal( node_ptr_t&& node_ptr )
        : var_signal::var_signal_base( std::move( node_ptr ) )
    {}

    // Copy assignment
    var_signal& operator=( const var_signal& ) = default;

    // Move assignment
    var_signal& operator=( var_signal&& other ) noexcept
    {
        var_signal::signal_base::operator=( std::move( other ) );
        return *this;
    }

    ~var_signal() = default;

    void set( const S& new_value ) const
    {
        var_signal::signal_base::set_value( new_value );
    }

    void set( S&& new_value ) const
    {
        var_signal::var_signal_base::set_value( std::move( new_value ) );
    }

    const var_signal& operator<<=( const S& new_value ) const
    {
        var_signal::var_signal_base::set_value( new_value );
        return *this;
    }

    const var_signal& operator<<=( S&& new_value ) const
    {
        var_signal::var_signal_base::set_value( std::move( new_value ) );
        return *this;
    }

    template <typename F>
    void modify( const F& func ) const
    {
        var_signal::var_signal_base::modify_value( func );
    }
};

// Specialize for references
template <typename S>
class var_signal<S&> : public detail::var_signal_base<std::reference_wrapper<S>>
{
private:
    using node_t = ::ureact::detail::var_node<std::reference_wrapper<S>>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
    using value_t = S;

    // Default ctor
    var_signal() = default;

    // Copy ctor
    var_signal( const var_signal& ) = default;

    // Move ctor
    var_signal( var_signal&& other ) noexcept
        : var_signal::signal( std::move( other ) )
    {}

    // Node ctor
    explicit var_signal( node_ptr_t&& node_ptr )
        : var_signal::signal( std::move( node_ptr ) )
    {}

    // Copy assignment
    var_signal& operator=( const var_signal& ) = default;

    // Move assignment
    var_signal& operator=( var_signal&& other ) noexcept
    {
        var_signal::signal::operator=( std::move( other ) );
        return *this;
    }

    ~var_signal() = default;

    void set( std::reference_wrapper<S> new_value ) const
    {
        var_signal::var_signal_base::set_value( new_value );
    }

    const var_signal& operator<<=( std::reference_wrapper<S> new_value ) const
    {
        var_signal::var_signal_base::set_value( new_value );
        return *this;
    }
};

} // namespace ureact
