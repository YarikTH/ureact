//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_SIGNAL_HPP
#define UREACT_SIGNAL_HPP

#include <ureact/equal_to.hpp>
#include <ureact/type_traits.hpp>
#include <ureact/ureact.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
class signal_node : public observable_node
{
public:
    explicit signal_node( context& context )
        : observable_node( context )
    {}

    template <typename T>
    signal_node( context& context, T&& value )
        : observable_node( context )
        , m_value( std::forward<T>( value ) )
    {}

    UREACT_WARN_UNUSED_RESULT const S& value_ref() const
    {
        return m_value;
    }

    // Assign a new value and do pulse only if new value is different from the current one
    template <class T>
    void pulse_if_value_changed( T&& new_value )
    {
        if( !equal_to( this->m_value, new_value ) )
        {
            this->m_value = std::forward<T>( new_value );
            this->get_graph().on_node_pulse( *this );
        }
    }

    // Perform pulse after value modification was performed
    void pulse_after_modify()
    {
        this->get_graph().on_node_pulse( *this );
    }

protected:
    S m_value;
};

template <typename S>
class var_node final
    : public signal_node<S>
    , public input_node_interface
{
public:
    template <typename T>
    explicit var_node( context& context, T&& value )
        : var_node::signal_node( context, std::forward<T>( value ) )
        , m_new_value( value )
    {}

    // LCOV_EXCL_START
    void tick( turn_type& ) override
    {
        assert( false && "Ticked var_node" );
    }
    // LCOV_EXCL_STOP

    template <typename V>
    void set_value( V&& new_value )
    {
        m_new_value = std::forward<V>( new_value );

        m_is_input_added = true;

        // m_is_input_added takes precedences over m_is_input_modified
        // the only difference between the two is that m_is_input_modified doesn't/can't compare
        m_is_input_modified = false;
    }

    // This is signal-specific
    template <typename F>
    void modify_value( F& func )
    {
        // There hasn't been any set(...) input yet, modify.
        if( !m_is_input_added )
        {
            std::invoke( func, this->m_value );

            m_is_input_modified = true;
        }
        // There's a new_value, modify new_value instead.
        // The modified new_value will be handled like before, i.e. it'll be compared to m_value
        // in apply_input
        else
        {
            std::invoke( func, m_new_value );
        }
    }

    bool apply_input( turn_type& ) override
    {
        if( m_is_input_added )
        {
            m_is_input_added = false;

            if( !equal_to( this->m_value, m_new_value ) )
            {
                this->m_value = std::move( m_new_value );
                this->get_graph().on_input_change( *this );
                return true;
            }
            return false;
        }
        if( m_is_input_modified )
        {
            m_is_input_modified = false;

            this->get_graph().on_input_change( *this );
            return true;
        }
        return false;
    }

private:
    S m_new_value;
    bool m_is_input_added = false; // TODO: replace 2 bools with enum class
    bool m_is_input_modified = false;
};

template <typename S>
class signal_base : public reactive_base<signal_node<S>>
{
public:
    signal_base() = default;

    template <typename Node>
    explicit signal_base( std::shared_ptr<Node>&& node )
        : signal_base::reactive_base( std::move( node ) )
    {}

protected:
    UREACT_WARN_UNUSED_RESULT const S& get_value() const
    {
        assert( !this->m_node->get_graph().is_locked() && "Can't read signal value from callback" );
        return this->m_node->value_ref();
    }

    template <typename T>
    void set_value( T&& new_value ) const
    {
        auto node_ptr = get_var_node();
        auto& graph_ref = node_ptr->get_graph();
        assert( !graph_ref.is_locked() && "Can't set signal value from callback" );

        graph_ref.push_input( node_ptr,
            [node_ptr, &new_value] { node_ptr->set_value( std::forward<T>( new_value ) ); } );
    }

    template <typename F>
    void modify_value( const F& func ) const
    {
        auto node_ptr = get_var_node();
        auto& graph_ref = node_ptr->get_graph();
        assert( !graph_ref.is_locked() && "Can't modify signal value from callback" );

        graph_ref.push_input( node_ptr, [node_ptr, &func] { node_ptr->modify_value( func ); } );
    }

private:
    UREACT_WARN_UNUSED_RESULT auto get_var_node() const
    {
        return static_cast<var_node<S>*>( this->m_node.get() );
    }
};

} // namespace detail

/*!
 * @brief Reactive variable that can propagate its changes to dependents and react to changes of
 * its dependencies
 *
 *  A signal is a reactive variable that can propagate its changes to dependents
 *  and react to changes of its dependencies.
 *
 *  Instances of this class act as a proxies to signal nodes. It takes shared
 *  ownership of the node, so while it exists, the node will not be destroyed.
 *  Copy, move and assignment semantics are similar to std::shared_ptr.
 */
template <typename S>
class signal : public detail::signal_base<S>
{
protected:
    using Node = detail::signal_node<S>;

public:
    /*!
     * @brief Alias to value type to use in metaprogramming
     */
    using value_t = S;

    /*!
     * @brief Default construct @ref signal
     *
     * Default constructed @ref signal is not attached to node, so it is not valid
     */
    signal() = default;

    /*!
     * @brief Construct from the given node
     */
    explicit signal( std::shared_ptr<Node>&& node )
        : signal::signal_base( std::move( node ) )
    {}

    /*!
     * @brief Return value of linked node
     */
    UREACT_WARN_UNUSED_RESULT const S& get() const
    {
        assert( this->is_valid() && "Can't get value of signal not attached to a node" );
        return this->get_value();
    }

    /*!
     * @brief Return value of linked node
     */
    UREACT_WARN_UNUSED_RESULT const S& operator()() const
    {
        assert( this->is_valid() && "Can't get value of signal not attached to a node" );
        return this->get_value();
    }
};

/*!
 * @brief Source signals which values can be manually changed
 *
 *  This class extends the immutable signal interface with functions that support
 *  imperative value input. In the dataflow graph, input signals are sources.
 *  As such, they don't have any predecessors.
 */
template <typename S>
class var_signal : public signal<S>
{
private:
    using Node = detail::var_node<S>;

public:
    /*!
     * @brief Default construct @ref var_signal
     *
     * Default constructed @ref var_signal is not attached to node, so it is not valid.
     */
    var_signal() = default;

    /*!
     * @brief Construct from the given node
     */
    explicit var_signal( std::shared_ptr<Node>&& node )
        : var_signal::signal( std::move( node ) )
    {}

    /*!
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
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( new_value );
    }

    /*!
     * @brief Set new signal value
     *
     * Specialization of set(const S& new_value) for rvalue
     */
    void set( S&& new_value ) const
    {
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( std::move( new_value ) );
    }

    /*!
     * @brief Modify current signal value in-place
     *
     *  The signature of func should be equivalent to:
     *  * void func(const S&)
     *
     *  We can not compare the old and new values, we lose the ability to detect
     *  whether the data was actually changed. We always have to assume that
     *  it did and re-calculate dependent signals.
     */
    template <typename F>
    void modify( const F& func ) const
    {
        static_assert(
            std::is_invocable_r_v<void, F, S&>, "Modifier functions should be void(S&)" );
        assert( this->is_valid() && "Can't modify value of var_signal not attached to a node" );
        this->modify_value( func );
    }

    /*!
     * @brief Set new signal value
     *
     *  Operator version of set(const S& new_value)
     */
    void operator<<=( const S& new_value ) const
    {
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( new_value );
    }

    /*!
     * @brief Set new signal value
     *
     *  Operator version of set(S&& new_value)
     *
     *  Specialization of operator<<=(const S& new_value) for rvalue
     */
    void operator<<=( S&& new_value ) const
    {
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( std::move( new_value ) );
    }

    /*!
     * @brief Modify current signal value in-place
     *
     *  Operator version of modify(const F& func)
     */
    template <typename F, class = std::enable_if_t<std::is_invocable_v<F, S&>>>
    void operator<<=( const F& func ) const
    {
        static_assert(
            std::is_invocable_r_v<void, F, S&>, "Modifier functions should be void(S&)" );
        assert( this->is_valid() && "Can't modify value of var_signal not attached to a node" );
        this->modify_value( func );
    }

    using signal<S>::operator();

    /*!
     * @brief Set new signal value
     *
     *  Function object version of set(const S& new_value)
     */
    void operator()( const S& new_value )
    {
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( new_value );
    }

    /*!
     * @brief Set new signal value
     *
     *  Function object version of set(S&& new_value)
     *
     *  Specialization of operator()(const S& new_value) for rvalue
     */
    void operator()( S&& new_value )
    {
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( std::move( new_value ) );
    }
};

/*!
 * @brief Interface for signal<S> that allows construction and assigment only for Owner class
 * 
 * member_signal is intended to be used as type for public members, so everybody can freely
 * access its public interface while being restricted from reassignment that should be allowed
 * only for the Owner class
 */
template <typename Owner, typename S>
class member_signal : public signal<S>
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_signal
     */
    member_signal() = default;

    /*!
     * @brief Copy construct from the given signal
     */
    member_signal( const signal<S>& src ) // NOLINT(google-explicit-constructor)
        : member_signal::signal( src )
    {}

    /*!
     * @brief Move construct from the given signal
     */
    member_signal( signal<S>&& src ) noexcept // NOLINT(google-explicit-constructor)
        : member_signal::signal( std::move( src ) )
    {}
};

/*!
 * @brief Interface for var_signal<S> that allows construction and assigment only for Owner class
 * 
 * member_var_signal is intended to be used as type for public members, so everybody can freely
 * access its public interface while being restricted from reassignment that should be allowed
 * only for the Owner class
 */
template <typename Owner, typename S>
class member_var_signal : public var_signal<S>
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_signal
     */
    member_var_signal() = default;

    /*!
     * @brief Copy construct from the given var_signal
     */
    member_var_signal( const var_signal<S>& src ) // NOLINT(google-explicit-constructor)
        : member_var_signal::var_signal( src )
    {}

    /*!
     * @brief Move construct from the given var_signal
     */
    member_var_signal( var_signal<S>&& src ) noexcept // NOLINT(google-explicit-constructor)
        : member_var_signal::var_signal( std::move( src ) )
    {}
};

/// Base class to setup aliases to member signal classes with specific owner class
template <class Owner>
class member_signal_user
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_signal_user
     */
    member_signal_user() = default;

    template <class S>
    using member_signal = member_signal<Owner, S>;

    template <class S>
    using member_var_signal = member_var_signal<Owner, S>;
};

/// Macro to setup aliases to member signal classes with specific owner class
#define UREACT_USE_MEMBER_SIGNALS( Owner )                                                         \
    template <class S>                                                                             \
    using member_signal = ::ureact::member_signal<Owner, S>;                                       \
    template <class S>                                                                             \
    using member_var_signal = ::ureact::member_var_signal<Owner, S>


namespace detail
{

template <typename S, typename V>
UREACT_WARN_UNUSED_RESULT auto make_var_signal( context& context, V&& v )
{
    return var_signal<S>{ std::make_shared<var_node<S>>( context, std::forward<V>( v ) ) };
}

template <typename V, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto make_var_impl( context& context, V&& v )
{
    // TODO: detect var_signal type and then construct it once
    //       unfortunately it is not directly possible with select_t and std::condition
    //       because all branches should be well-formed, that is not the case with
    //       attempt to access "typename S::value_t" from types without value_t member
    //       https://stackoverflow.com/questions/24098278/stdconditional-compile-time-branch-evaluation
    if constexpr( is_observable_v<S> )
    {
        // clang-format off
        using S2 =
            select_t<
                condition<is_signal_v<S>,   signal<typename S::value_t>>,
                condition<is_event_v<S>,    events<typename S::value_t>>,
                S>;
        // clang-format on

        return make_var_signal<S2>( context, std::forward<V>( v ) );
    }
    else
    {
        return make_var_signal<S>( context, std::forward<V>( v ) );
    }
}

} // namespace detail

/*!
 * @brief Create a new input signal node and links it to the returned var_signal instance
 */
template <typename V>
UREACT_WARN_UNUSED_RESULT auto make_var( context& context, V&& value )
{
    assert( !_get_internals( context ).get_graph().is_locked() && "Can't make var from callback" );
    return make_var_impl( context, std::forward<V>( value ) );
}

/*!
 * @brief Create a new signal node and links it to the returned signal instance
 *
 *  Returned value doesn't have input interface and can be used a placeholder where signal is required.
 *  Currently it's just cosmetic function that can be expressed as signal{ make_var(context, value) }
 *  but it can be optimized in future.
 */
template <typename V, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto make_const( context& context, V&& value ) -> signal<S>
{
    assert(
        !_get_internals( context ).get_graph().is_locked() && "Can't make const from callback" );
    return make_var_impl( context, std::forward<V>( value ) );
}

/*!
 * @brief A wrapper type for a tuple of signal references
 * @tparam Values types of signal values
 *
 *  Created with @ref with()
 */
template <typename... Values>
class signal_pack final
{
public:
    /*!
     * @brief Class to store signals instead of signal references
     */
    class stored
    {
    public:
        /*!
         * @brief Construct from signals
         */
        explicit stored( const signal<Values>&... deps )
            : data( std::tie( deps... ) )
        {}

        /*!
         * @brief The wrapped tuple
         */
        std::tuple<signal<Values>...> data;
    };

    /*!
     * @brief Construct from signals
     */
    explicit signal_pack( const signal<Values>&... deps )
        : data( std::tie( deps... ) )
    {}

    /*!
     * @brief Construct from stored signals
     */
    explicit signal_pack( const stored& value )
        : data( std::apply(
            []( const signal<Values>&... deps ) { return std::tie( deps... ); }, value.data ) )
    {}

    /*!
     * @brief Convert signal references to signals so they can be stored
     */
    UREACT_WARN_UNUSED_RESULT stored store() const
    {
        return std::apply(
            []( const signal<Values>&... deps ) { return stored{ deps... }; }, data );
    }

    /*!
     * @brief The wrapped tuple
     */
    std::tuple<const signal<Values>&...> data;
};

/*!
 * @brief Utility function to create a signal_pack from given signals
 * @tparam Values types of signal values
 *
 *  Creates a signal_pack from the signals passed as deps.
 *  Semantically, this is equivalent to std::tie.
 */
template <typename... Values>
UREACT_WARN_UNUSED_RESULT auto with( const signal<Values>&... deps )
{
    return signal_pack<Values...>( deps... );
}

UREACT_END_NAMESPACE

#endif //UREACT_SIGNAL_HPP