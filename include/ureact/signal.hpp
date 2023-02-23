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

#include <ureact/context.hpp>
#include <ureact/detail/observable_node.hpp>
#include <ureact/equal_to.hpp>
#include <ureact/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
class signal_node : public observable_node
{
public:
    explicit signal_node( const context& context )
        : observable_node( context )
    {}

    template <typename T>
    signal_node( const context& context, T&& value )
        : observable_node( context )
        , m_value( std::forward<T>( value ) )
    {}

    UREACT_WARN_UNUSED_RESULT const S& value_ref() const
    {
        return m_value;
    }

    // Assign a new value if is differed and return if updated
    template <class T>
    UREACT_WARN_UNUSED_RESULT update_result try_change_value( T&& new_value )
    {
        if( !equal_to( this->m_value, new_value ) )
        {
            this->m_value = std::forward<T>( new_value );
            return update_result::changed;
        }
        return update_result::unchanged;
    }

protected:
    S m_value;
};

template <typename S>
class var_node final : public signal_node<S>
{
public:
    template <typename T>
    explicit var_node( const context& context, T&& value )
        : var_node::signal_node( context, std::forward<T>( value ) )
        , m_new_value( value )
    {}

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

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        if( m_is_input_added )
        {
            m_is_input_added = false;

            if( !equal_to( this->m_value, m_new_value ) )
            {
                this->m_value = std::move( m_new_value );
                return update_result::changed;
            }
            return update_result::unchanged;
        }
        if( m_is_input_modified )
        {
            m_is_input_modified = false;

            return update_result::changed;
        }
        return update_result::unchanged;
    }

private:
    S m_new_value;
    bool m_is_input_added = false; // TODO: replace 2 bools with enum class
    bool m_is_input_modified = false;
};

template <typename S>
class signal_internals
{
public:
    signal_internals() = default;

    template <typename Node>
    explicit signal_internals( std::shared_ptr<Node>&& node )
        : m_node( std::move( node ) )
    {}

    UREACT_WARN_UNUSED_RESULT std::shared_ptr<signal_node<S>>& get_node_ptr()
    {
        return m_node;
    }

    UREACT_WARN_UNUSED_RESULT const std::shared_ptr<signal_node<S>>& get_node_ptr() const
    {
        return m_node;
    }

    UREACT_WARN_UNUSED_RESULT node_id get_node_id() const
    {
        return m_node->get_node_id();
    }

    UREACT_WARN_UNUSED_RESULT const S& get_value() const
    {
        assert( !this->m_node->get_graph().is_locked() && "Can't read signal value from callback" );
        return this->m_node->value_ref();
    }

protected:
    template <typename T>
    void set_value( T&& new_value ) const
    {
        auto node_ptr = get_var_node();
        auto& graph_ref = node_ptr->get_graph();
        assert( !graph_ref.is_locked() && "Can't set signal value from callback" );
        node_ptr->set_value( std::forward<T>( new_value ) );
        graph_ref.push_input( node_ptr->get_node_id() );
    }

    template <typename F>
    void modify_value( const F& func ) const
    {
        auto node_ptr = get_var_node();
        auto& graph_ref = node_ptr->get_graph();
        assert( !graph_ref.is_locked() && "Can't modify signal value from callback" );
        node_ptr->modify_value( func );
        graph_ref.push_input( node_ptr->get_node_id() );
    }

private:
    UREACT_WARN_UNUSED_RESULT auto get_var_node() const
    {
        return static_cast<var_node<S>*>( this->m_node.get() );
    }

    std::shared_ptr<signal_node<S>> m_node;
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
class signal : protected detail::signal_internals<S>
{
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
     * @brief Return @ref context used by attached node
     */
    UREACT_WARN_UNUSED_RESULT context& get_context()
    {
        return this->get_node_ptr()->get_context();
    }

    /*!
     * @brief Return @ref context used by attached node
     */
    UREACT_WARN_UNUSED_RESULT const context& get_context() const
    {
        return this->get_node_ptr()->get_context();
    }

    /*!
     * @brief Tests if this instance is linked to a node
     */
    UREACT_WARN_UNUSED_RESULT bool is_valid() const
    {
        return this->get_node_ptr() != nullptr;
    }

    /*!
     * @brief Equally compare with other signal
     * 
     * Semantic equivalent of operator==
     * It is intended to allow overload of operator== to make new signal
     */
    UREACT_WARN_UNUSED_RESULT bool equal_to( const signal& other ) const
    {
        return this->get_node_ptr() == other.get_node_ptr();
    }

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

    /*!
     * @brief Pointer access value of linked node
     * 
     * TODO: check it in tests
     */
    UREACT_WARN_UNUSED_RESULT const S* operator->() const
    {
        assert( this->is_valid() && "Can't access value of signal not attached to a node" );
        return &this->get_value();
    }

    /*!
     * @brief Return internals. Not intended to use in user code
     */
    UREACT_WARN_UNUSED_RESULT friend detail::signal_internals<S>& get_internals( signal<S>& s )
    {
        return s;
    }

    /*!
     * @brief Return internals. Not intended to use in user code
     */
    UREACT_WARN_UNUSED_RESULT friend const detail::signal_internals<S>& get_internals(
        const signal<S>& s )
    {
        return s;
    }

protected:
    using Node = detail::signal_node<S>;

    /*!
     * @brief Construct from the given node
     */
    explicit signal( std::shared_ptr<Node>&& node )
        : signal::signal_internals( std::move( node ) )
    {}

    template <typename Ret, typename Node, typename... Args>
    friend Ret detail::create_wrapped_node( Args&&... args );
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
public:
    /*!
     * @brief Default construct @ref var_signal
     *
     * Default constructed @ref var_signal is not attached to node, so it is not valid.
     */
    var_signal() = default;

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

protected:
    using Node = detail::var_node<S>;

    /*!
     * @brief Construct from the given node
     */
    explicit var_signal( std::shared_ptr<Node>&& node )
        : var_signal::signal( std::move( node ) )
    {}

    template <typename Ret, typename Node, typename... Args>
    friend Ret detail::create_wrapped_node( Args&&... args );
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

template <typename V, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto make_var_impl( const context& context, V&& v )
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

        return detail::create_wrapped_node<var_signal<S2>, var_node<S2>>(
            context, std::forward<V>( v ) );
    }
    else
    {
        return detail::create_wrapped_node<var_signal<S>, var_node<S>>(
            context, std::forward<V>( v ) );
    }
}

} // namespace detail

/*!
 * @brief Create a new input signal node and links it to the returned var_signal instance
 */
template <typename V>
UREACT_WARN_UNUSED_RESULT auto make_var( const context& context, V&& value )
{
    assert( !get_internals( context ).get_graph().is_locked() && "Can't make var from callback" );
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
UREACT_WARN_UNUSED_RESULT auto make_const( const context& context, V&& value ) -> signal<S>
{
    assert( !get_internals( context ).get_graph().is_locked() && "Can't make const from callback" );
    return make_var_impl( context, std::forward<V>( value ) );
}

UREACT_END_NAMESPACE

#endif //UREACT_SIGNAL_HPP
