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
    signal_base() = default;

    template <typename T>
    explicit signal_base( T&& t )
        : signal_base::reactive_base( std::forward<T>( t ) )
    {}

private:
    auto get_var_node() const -> var_node<S>*
    {
        return static_cast<var_node<S>*>(this->m_ptr.get());
    }
    
protected:
    const S& get_value() const
    {
        return this->m_ptr->value_ref();
    }
    
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

/**
 * A signal is a reactive variable that can propagate its changes to dependents
 * and react to changes of its dependencies.
 *
 * Instances of this class act as a proxies to signal nodes. It takes shared
 * ownership of the node, so while it exists, the node will not be destroyed.
 * Copy, move and assignment semantics are similar to std::shared_ptr.
 *
 * signals are created by constructor functions, i.e. make_signal.
 *
 * Specialization for non-reference types.
 */
template <typename S>
class signal : public detail::signal_base<S>
{
private:
    using node_t = detail::signal_node<S>;

public:
    using value_t = S;

    /// Default constructor that needed for REACTIVE_REF for some reason
    /// @todo investigate and remove it if possible
    signal() = default;

    /**
     * Construct temp_signal from var_node.
     * @todo make it private and allow to call it only from make_var function
     */
    explicit signal( std::shared_ptr<node_t>&& node_ptr )
        : signal::signal_base( std::move( node_ptr ) )
    {}

    /// Return value of linked node
    const S& value() const
    {
        return signal::get_value();
    }

    /// Semantically equivalent to the respective free function in namespace ureact.
    S flatten() const
    {
        static_assert( is_signal<S>::value, "flatten requires a signal value type." );
        return ::ureact::flatten( *this );
    }
};

/**
 * A signal is a reactive variable that can propagate its changes to dependents
 * and react to changes of its dependencies.
 *
 * Instances of this class act as a proxies to signal nodes. It takes shared
 * ownership of the node, so while it exists, the node will not be destroyed.
 * Copy, move and assignment semantics are similar to std::shared_ptr.
 *
 * signals are created by constructor functions, i.e. make_signal.
 *
 * Specialization for references.
 */
template <typename S>
class signal<S&> : public detail::signal_base<std::reference_wrapper<S>>
{
private:
    using node_t = detail::signal_node<std::reference_wrapper<S>>;

public:
    using value_t = S;

    /// Default constructor that needed for REACTIVE_REF for some reason
    /// @todo investigate and remove it if possible
    signal() = default;

    /**
     * Construct temp_signal from var_node.
     * @todo make it private and allow to call it only from make_var function
     */
    explicit signal( std::shared_ptr<node_t>&& node_ptr )
        : signal::signal_base( std::move( node_ptr ) )
    {}

    /// Return value of linked node
    const S& value() const
    {
        return signal::get_value();
    }
};

/**
 * This class extends the immutable signal interface with functions that support
 * imperative value input. In the dataflow graph, input signals are sources.
 * As such, they don't have any predecessors.
 *
 * var_signal is created by constructor function make_var.
 *
 * Specialization for non-reference types.
 */
template <typename S>
class var_signal : public signal<S>
{
private:
    using node_t = ::ureact::detail::var_node<S>;

public:
    /**
     * Construct var_signal from var_node.
     * @todo make it private and allow to call it only from make_var function
     */
    explicit var_signal( std::shared_ptr<node_t>&& node_ptr )
        : var_signal::signal( std::move( node_ptr ) )
    {}

    /**
     * @brief Set new signal value
     *
     * Set the the signal value of the linked variable signal node to a new_value.
     * If the old value equals the new value, the call has no effect.
     *
     * Furthermore, if set was called inside of a transaction function, it will
     * return after the changed value has been set and change propagation is delayed
     * until the transaction function returns.
     * Otherwise, propagation starts immediately and Set blocks until it's done.
     */
    void set( const S& new_value ) const
    {
        var_signal::set_value( new_value );
    }

    /// @copydoc set
    void set( S&& new_value ) const
    {
        var_signal::set_value( std::move( new_value ) );
    }

    /**
     * @brief Operator version of set()
     *
     * Semantically equivalent to set().
     */
    const var_signal& operator<<=( const S& new_value ) const
    {
        var_signal::set_value( new_value );
        return *this;
    }

    /// @copydoc operator<<=
    const var_signal& operator<<=( S&& new_value ) const
    {
        var_signal::set_value( std::move( new_value ) );
        return *this;
    }

    /**
     * @brief Modify current signal value in-place
     */
    template <typename F>
    void modify( const F& func ) const
    {
        var_signal::modify_value( func );
    }
};


/**
 * This class extends the immutable signal interface with functions that support
 * imperative value input. In the dataflow graph, input signals are sources.
 * As such, they don't have any predecessors.
 *
 * var_signal is created by constructor function make_var.
 *
 * Specialization for references.
 */
template <typename S>
class var_signal<S&> : public signal<std::reference_wrapper<S>>
{
private:
    using node_t = detail::var_node<std::reference_wrapper<S>>;

public:
    using value_t = S;
    
    /**
     * Construct var_signal from var_node.
     * @todo make it private and allow to call it only from make_var function
     */
    explicit var_signal( std::shared_ptr<node_t>&& node_ptr )
        : var_signal::signal( std::move( node_ptr ) )
    {}

    /**
     * @brief Set new signal value
     *
     * Set the the signal value of the linked variable signal node to a new_value.
     * If the old value equals the new value, the call has no effect.
     *
     * Furthermore, if set was called inside of a transaction function, it will
     * return after the changed value has been set and change propagation is delayed
     * until the transaction function returns.
     * Otherwise, propagation starts immediately and Set blocks until it's done.
     */
    void set( std::reference_wrapper<S> new_value ) const
    {
        var_signal::set_value( new_value );
    }

    /**
     * @brief Operator version of set()
     *
     * Semantically equivalent to set().
     */
    const var_signal& operator<<=( std::reference_wrapper<S> new_value ) const
    {
        var_signal::set_value( new_value );
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
    /**
     * Construct temp_signal from var_node.
     * @todo make it private and allow to call it only from make_var function
     */
    explicit temp_signal( std::shared_ptr<node_t>&& ptr )
        : temp_signal::signal( std::move( ptr ) )
    {}

    /// Return internal operator, leaving node invalid
    op_t steal_op()
    {
        auto* node_ptr = static_cast<node_t*>( this->m_ptr.get() );
        return node_ptr->steal_op();
    }
};

} // namespace detail

} // namespace ureact
