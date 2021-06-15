#pragma once

#include "ureact/detail/graph/signal_node.hpp"
#include "ureact/detail/graph/signal_op_node.hpp"
#include "ureact/detail/graph/function_op.hpp"
#include "ureact/detail/graph/flatten_node.hpp"
#include "ureact/detail/graph/var_node.hpp"

namespace ureact
{
// Forward declaration to break cyclic dependency
template <typename S>
class signal;
template <typename S>
class var_signal;
template <typename inner_value_t>
auto flatten( const signal<signal<inner_value_t>>& outer ) -> signal<inner_value_t>;

namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// reactive_base
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename node_t>
class reactive_base
{
public:
    reactive_base() = default;

    reactive_base( std::shared_ptr<node_t>&& ptr ) noexcept
        : m_ptr( std::move( ptr ) )
    {}

    bool is_valid() const
    {
        return m_ptr != nullptr;
    }

    bool equals( const reactive_base& other ) const
    {
        return this->m_ptr == other.m_ptr;
    }

    context& get_context() const
    {
        return m_ptr->get_context();
    }

protected:
    std::shared_ptr<node_t> m_ptr;

    template <typename node_t_>
    friend const std::shared_ptr<node_t_>& get_node_ptr( const reactive_base<node_t_>& node );
};

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
        return static_cast<var_node<S>*>( this->m_ptr.get() );
    }

protected:
    const S& get_value() const
    {
        return this->m_ptr->value_ref();
    }

    template <typename T>
    void set_value( T&& new_value ) const
    {
        get_var_node()->request_add_input( std::forward<T>( new_value ) );
    }

    template <typename F>
    void modify_value( const F& func ) const
    {
        get_var_node()->request_modify_input( func );
    }
};

// Got from https://stackoverflow.com/a/34672753
// std::is_base_of for template classes
template <template <typename...> class base, typename derived>
struct is_base_of_template_impl
{
    template <typename... Ts>
    static constexpr std::true_type test( const base<Ts...>* )
    {
        return {};
    }
    static constexpr std::false_type test( ... )
    {
        return {};
    }
    using type = decltype( test( std::declval<derived*>() ) );
};

template <template <typename...> class base, typename derived>
using is_base_of_template = typename is_base_of_template_impl<base, derived>::type;

} // namespace detail

/// Return if type is signal or its inheritor
template <typename T>
struct is_signal : detail::is_base_of_template<signal, T>
{};

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

template <typename V,
    typename S = typename std::decay<V>::type,
    class = typename std::enable_if<!is_signal<S>::value>::type>
auto make_var_impl( context& context, V&& value ) -> var_signal<S>
{
    return var_signal<S>(
        std::make_shared<::ureact::detail::var_node<S>>( context, std::forward<V>( value ) ) );
}

template <typename S>
auto make_var_impl( context& context, std::reference_wrapper<S> value ) -> var_signal<S&>
{
    return var_signal<S&>(
        std::make_shared<::ureact::detail::var_node<std::reference_wrapper<S>>>( context, value ) );
}

template <typename V,
    typename S = typename std::decay<V>::type,
    typename inner_t = typename S::value_t,
    class = typename std::enable_if<is_signal<S>::value>::type>
auto make_var_impl( context& context, V&& value ) -> var_signal<signal<inner_t>>
{
    return var_signal<signal<inner_t>>(
        std::make_shared<::ureact::detail::var_node<signal<inner_t>>>(
            context, std::forward<V>( value ) ) );
}

} // namespace detail

template <typename V>
auto make_var( context& context, V&& value )
    -> decltype( detail::make_var_impl( context, std::forward<V>( value ) ) )
{
    return detail::make_var_impl( context, std::forward<V>( value ) );
}

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

template <typename S, typename op_t, typename... Args>
auto make_temp_signal( context& context, Args&&... args ) -> temp_signal<S, op_t>
{
    return temp_signal<S, op_t>(
        std::make_shared<signal_op_node<S, op_t>>( context, std::forward<Args>( args )... ) );
}

} // namespace detail

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_pack - Wraps several nodes in a tuple. Create with comma operator.
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... values_t>
class signal_pack
{
public:
    explicit signal_pack( const signal<values_t>&... deps )
        : data( std::tie( deps... ) )
    {}

    template <typename... cur_values_t, typename append_value_t>
    signal_pack(
        const signal_pack<cur_values_t...>& cur_args, const signal<append_value_t>& new_arg )
        : data( std::tuple_cat( cur_args.data, std::tie( new_arg ) ) )
    {}

    std::tuple<const signal<values_t>&...> data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// with - Utility function to create a signal_pack
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... values_t>
auto with( const signal<values_t>&... deps ) -> signal_pack<values_t...>
{
    return signal_pack<values_t...>( deps... );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Comma operator overload to create signal pack from 2 signals.
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename left_val_t, typename right_val_t>
auto operator,( const signal<left_val_t>& a, const signal<right_val_t>& b )
                  -> signal_pack<left_val_t, right_val_t>
{
    return signal_pack<left_val_t, right_val_t>( a, b );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Comma operator overload to append node to existing signal pack.
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... cur_values_t, typename append_value_t>
auto operator,( const signal_pack<cur_values_t...>& cur, const signal<append_value_t>& append )
                  -> signal_pack<cur_values_t..., append_value_t>
{
    return signal_pack<cur_values_t..., append_value_t>( cur, append );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// make_signal
///////////////////////////////////////////////////////////////////////////////////////////////////
// Single arg
template <typename value_t,
    typename in_f,
    typename F = typename std::decay<in_f>::type,
    typename S = typename std::result_of<F( value_t )>::type,
    typename op_t
    = ::ureact::detail::function_op<S, F, ::ureact::detail::signal_node_ptr_t<value_t>>>
auto make_signal( const signal<value_t>& arg, in_f&& func ) -> detail::temp_signal<S, op_t>
{
    context& context = arg.get_context();

    return detail::make_temp_signal<S, op_t>(
        context, std::forward<in_f>( func ), get_node_ptr( arg ) );
}

// Multiple args
template <typename... values_t,
    typename in_f,
    typename F = typename std::decay<in_f>::type,
    typename S = typename std::result_of<F( values_t... )>::type,
    typename op_t
    = ::ureact::detail::function_op<S, F, ::ureact::detail::signal_node_ptr_t<values_t>...>>
auto make_signal( const signal_pack<values_t...>& arg_pack, in_f&& func )
    -> detail::temp_signal<S, op_t>
{
    struct node_builder
    {
        explicit node_builder( context& context, in_f&& func )
            : m_context( context )
            , m_my_func( std::forward<in_f>( func ) )
        {}

        auto operator()( const signal<values_t>&... args ) -> detail::temp_signal<S, op_t>
        {
            return detail::make_temp_signal<S, op_t>(
                m_context, std::forward<in_f>( m_my_func ), get_node_ptr( args )... );
        }

        context& m_context;
        in_f m_my_func;
    };

    return apply(
        node_builder( std::get<0>( arg_pack.data ).get_context(), std::forward<in_f>( func ) ),
        arg_pack.data );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// operator->* overload to connect signals to a function and return the resulting signal.
///////////////////////////////////////////////////////////////////////////////////////////////////
// Single arg
template <typename F,
    template <typename>
    class signal_t,
    typename value_t,
    class = typename std::enable_if<is_signal<signal_t<value_t>>::value>::type>
auto operator->*( const signal_t<value_t>& arg, F&& func )
    -> signal<typename std::result_of<F( value_t )>::type>
{
    return ::ureact::make_signal( arg, std::forward<F>( func ) );
}

// Multiple args
template <typename F, typename... values_t>
auto operator->*( const signal_pack<values_t...>& arg_pack, F&& func )
    -> signal<typename std::result_of<F( values_t... )>::type>
{
    return ::ureact::make_signal( arg_pack, std::forward<F>( func ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// flatten
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename inner_value_t>
auto flatten( const signal<signal<inner_value_t>>& outer ) -> signal<inner_value_t>
{
    context& context = outer.get_context();
    return signal<inner_value_t>(
        std::make_shared<::ureact::detail::flatten_node<signal<inner_value_t>, inner_value_t>>(
            context, get_node_ptr( outer ), get_node_ptr( outer.value() ) ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// decay_input
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @todo understand its meaning and document it
template <typename T>
struct decay_input
{
    using type = T;
};

template <typename T>
struct decay_input<var_signal<T>>
{
    using type = signal<T>;
};

namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// identity (workaround to enable enable decltype()::X)
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct identity
{
    using type = T;
};

} // namespace detail

///////////////////////////////////////////////////////////////////////////////////////////////////
/// flatten macros
///////////////////////////////////////////////////////////////////////////////////////////////////

#define REACTIVE_REF( obj, name )                                                                  \
    flatten( make_signal(                                                                          \
        obj, []( const typename ::ureact::detail::identity<decltype( obj )>::type::value_t& r ) {  \
            using T = decltype( r.name );                                                          \
            using S = typename ::ureact::decay_input<T>::type;                                     \
            return static_cast<S>( r.name );                                                       \
        } ) )

#define REACTIVE_PTR( obj, name )                                                                  \
    flatten( make_signal(                                                                          \
        obj, []( typename ::ureact::detail::identity<decltype( obj )>::type::value_t r ) {         \
            assert( r != nullptr );                                                                \
            using T = decltype( r->name );                                                         \
            using S = typename ::ureact::decay_input<T>::type;                                     \
            return static_cast<S>( r->name );                                                      \
        } ) )

} // namespace ureact
