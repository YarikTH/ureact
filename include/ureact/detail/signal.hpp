#pragma once

#include "ureact/detail/flatten.hpp"
#include "ureact/detail/type_traits.hpp"
#include "ureact/detail/reactive_input.hpp"
#include "ureact/detail/graph/signal_node.hpp"
#include "ureact/detail/graph/signal_op_node.hpp"
#include "ureact/detail/graph/var_node.hpp"

namespace ureact
{
namespace detail
{


///////////////////////////////////////////////////////////////////////////////////////////////////
/// reactive_base
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename node_t>
class reactive_base
{
public:
    // Default ctor
    reactive_base() = default;

    // Copy ctor
    reactive_base( const reactive_base& ) = default;

    // Move ctor
    reactive_base( reactive_base&& other ) noexcept = default;

    // Explicit node ctor
    reactive_base( std::shared_ptr<node_t>&& ptr ) noexcept
        : m_ptr( std::move( ptr ) )
    {}

    // Copy assignment
    reactive_base& operator=( const reactive_base& ) = default;

    // Move assignment
    reactive_base& operator=( reactive_base&& other ) noexcept = default;

    ~reactive_base() = default;

    bool is_valid() const
    {
        return m_ptr != nullptr;
    }

    bool equals( const reactive_base& other ) const
    {
        return this->m_ptr == other.m_ptr;
    }

    context* get_context() const
    {
        return m_ptr->get_context();
    }

protected:
    std::shared_ptr<node_t> m_ptr;

    template <typename node_t_>
    friend const std::shared_ptr<node_t_>& get_node_ptr( const reactive_base<node_t_>& node );
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// get_node_ptr
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename node_t>
const std::shared_ptr<node_t>& get_node_ptr( const reactive_base<node_t>& node )
{
    return node.m_ptr;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_base
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class signal_base : public reactive_base<signal_node<S>>
{
public:
    // Default ctor
    signal_base() = default;

    template <typename T>
    explicit signal_base( T&& t )
        : signal_base::reactive_base( std::forward<T>( t ) )
    {}

protected:
    const S& get_value() const
    {
        return this->m_ptr->value_ref();
    }
};

} // namespace detail

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class signal : public detail::signal_base<S>
{
private:
    using node_t = detail::signal_node<S>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
    using value_t = S;

    signal() = default;

    explicit signal( node_ptr_t&& node_ptr )
        : signal::signal_base( std::move( node_ptr ) )
    {}

    const S& value() const
    {
        return signal::signal_base::get_value();
    }

    S flatten() const
    {
        static_assert( is_signal<S>::value, "flatten requires a signal value type." );
        return ::ureact::flatten( *this );
    }
};

// Specialize for references
template <typename S>
class signal<S&> : public detail::signal_base<std::reference_wrapper<S>>
{
private:
    using node_t = detail::signal_node<std::reference_wrapper<S>>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
    using value_t = S;

    signal() = default;

    explicit signal( node_ptr_t&& node_ptr )
        : signal::signal_base( std::move( node_ptr ) )
    {}

    const S& value() const
    {
        return signal::signal_base::get_value();
    }
};


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
        get_var_node()->request_add_input(std::forward<T>(new_value));
    }

    template <typename F>
    void modify_value( const F& func ) const
    {
        get_var_node()->request_modify_input(func);
    }
};

} // namespace detail

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
    var_signal() = default;
    
    explicit var_signal( node_ptr_t&& node_ptr )
        : var_signal::var_signal_base( std::move( node_ptr ) )
    {}

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
    
    var_signal() = default;
    
    explicit var_signal( node_ptr_t&& node_ptr )
        : var_signal::var_signal_base( std::move( node_ptr ) )
    {}

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

namespace detail
{

/**
 * This class exposes additional type information of the linked node, which enables
 * r-value based node merging at construction time.
 * The primary use case for this is to avoid unnecessary nodes when creating signal
 * expression from overloaded arithmetic operators.
 *
 * temp_signal shouldn't be used as an l-value type, but instead implicitly
 * converted to signal.
 */
template <typename S, typename op_t>
class temp_signal : public signal<S>
{
private:
    using node_t = signal_op_node<S, op_t>;

public:
    explicit temp_signal( std::shared_ptr<node_t>&& ptr )
        : temp_signal::signal( std::move( ptr ) )
    {}

    op_t steal_op()
    {
        auto* node_ptr = static_cast<node_t*>( this->m_ptr.get() );
        return node_ptr->steal_op();
    }
};

} // namespace detail

} // namespace ureact
