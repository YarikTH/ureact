//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_EVENTS_HPP
#define UREACT_EVENTS_HPP

#include <ureact/ureact.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename E>
class event_stream_node : public observable_node
{
public:
    using data_t = std::vector<E>;

    explicit event_stream_node( context& context )
        : observable_node( context )
    {}

    void set_current_turn( const turn_type& turn )
    {
        if( m_cur_turn_id != turn )
        {
            m_cur_turn_id = turn;
            m_events.clear();
        }
    }

    void set_current_turn_force_update( const turn_type& turn )
    {
        m_cur_turn_id = turn;
        m_events.clear();
    }

    void set_current_turn_force_update_no_clear( const turn_type& turn )
    {
        m_cur_turn_id = turn;
    }

    data_t& events()
    {
        return m_events;
    }

    void pulse_if_has_events()
    {
        if( !m_events.empty() )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

protected:
    data_t m_events;

private:
    unsigned m_cur_turn_id{ std::numeric_limits<unsigned>::max() };
};

template <typename E>
class event_source_node final
    : public event_stream_node<E>
    , public input_node_interface
{
public:
    explicit event_source_node( context& context )
        : event_source_node::event_stream_node( context )
    {}

    ~event_source_node() override = default;

    // LCOV_EXCL_START
    void tick( turn_type& ) override
    {
        assert( false && "Ticked event_source_node" );
    }
    // LCOV_EXCL_STOP

    template <typename V>
    void emit_value( V&& v )
    {
        // Clear input from previous turn
        if( m_changed_flag )
        {
            m_changed_flag = false;
            this->m_events.clear();
        }

        this->m_events.push_back( std::forward<V>( v ) );
    }

    bool apply_input( turn_type& turn ) override
    {
        if( this->m_events.size() > 0 && !m_changed_flag )
        {
            this->set_current_turn_force_update_no_clear( turn );
            m_changed_flag = true;
            this->get_graph().on_input_change( *this );
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    bool m_changed_flag = false;
};

template <typename E, typename Op>
class event_op_node final : public event_stream_node<E>
{
public:
    template <typename... Args>
    explicit event_op_node( context& context, Args&&... args )
        : event_op_node::event_stream_node( context )
        , m_op( std::forward<Args>( args )... )
    {
        m_op.attach( *this );
    }

    ~event_op_node() override
    {
        if( !m_was_op_stolen )
        {
            m_op.detach( *this );
        }
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );

        m_op.collect( turn, event_collector( this->m_events ) );

        this->pulse_if_has_events();
    }

    UREACT_WARN_UNUSED_RESULT Op steal_op()
    {
        assert( !m_was_op_stolen && "Op was already stolen" );
        m_was_op_stolen = true;
        m_op.detach( *this );
        return std::move( m_op );
    }

    UREACT_WARN_UNUSED_RESULT bool was_op_stolen() const
    {
        return m_was_op_stolen;
    }

private:
    struct event_collector
    {
        using data_t = typename event_op_node::data_t;

        explicit event_collector( data_t& events )
            : m_events( events )
        {}

        void operator()( const E& e ) const
        {
            m_events.push_back( e );
        }

        data_t& m_events;
    };

    Op m_op;
    bool m_was_op_stolen = false;
};

template <typename E>
class event_stream_base : public reactive_base<event_stream_node<E>>
{
public:
    event_stream_base() = default;

    UREACT_MAKE_COPYABLE( event_stream_base );
    UREACT_MAKE_MOVABLE( event_stream_base );

    template <typename Node>
    explicit event_stream_base( std::shared_ptr<Node>&& node )
        : event_stream_base::reactive_base( std::move( node ) )
    {}

private:
    UREACT_WARN_UNUSED_RESULT auto get_event_source_node() const -> event_source_node<E>*
    {
        return static_cast<event_source_node<E>*>( this->m_node.get() );
    }

protected:
    template <typename T>
    void emit_event( T&& e ) const
    {
        auto node_ptr = get_event_source_node();
        auto& graph_ref = node_ptr->get_graph();
        assert( !graph_ref.is_locked() && "Can't emit event from callback" );

        graph_ref.push_input(
            node_ptr, [node_ptr, &e] { node_ptr->emit_value( std::forward<T>( e ) ); } );
    }
};

} // namespace detail

/*!
 * @brief Reactive event stream class
 *
 *  An instance of this class acts as a proxy to an event stream node.
 *  It takes shared ownership of the node, so while it exists, the node will not be destroyed.
 *  Copy, move and assignment semantics are similar to std::shared_ptr.
 */
template <typename E = unit>
class events : public detail::event_stream_base<E>
{
private:
    using Node = detail::event_stream_node<E>;

public:
    /*!
     * @brief Alias to value type to use in metaprogramming
     */
    using value_t = E;

    /*!
     * @brief Default construct @ref events
     *
     * Default constructed @ref events is not attached to node, so it is not valid
     */
    events() = default;

    /*!
     * @brief Construct from the given node
     */
    explicit events( std::shared_ptr<Node>&& node )
        : events::event_stream_base( std::move( node ) )
    {}
};

/*!
 * @brief @ref events that support imperative input
 *
 *  An event source extends the immutable @ref events interface with functions that support imperative input.
 */
template <typename E = unit>
class event_source : public events<E>
{
private:
    using Node = detail::event_source_node<E>;

public:
    class iterator;

    /*!
     * @brief Default construct @ref event_source
     *
     * Default constructed @ref event_source is not attached to node, so it is not valid
     */
    event_source() = default;

    /*!
     * @brief Construct from the given node
     */
    explicit event_source( std::shared_ptr<Node>&& node )
        : event_source::events( std::move( node ) )
    {}

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * If emit() was called inside of a transaction function, it will return after
     * the event has been queued and propagation is delayed until the transaction
     * function returns.
     * Otherwise, propagation starts immediately and emit() blocks until it’s done.
     */
    void emit( const E& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( e );
    }

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * Specialization of emit(const E& e) for rvalue
     */
    void emit( E&& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( std::move( e ) );
    }

    /*!
     * @brief Adds unit to the queue of outgoing events of the linked event source node
     *
     * Specialization of emit(const E& e) that allows to omit e value, when the emitted value is always @ref unit
     */
    void emit() const
    {
        static_assert( std::is_same_v<E, unit>, "Can't emit on non unit stream" );
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( unit{} );
    }

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * Function object version of emit(const E& e)
     */
    void operator()( const E& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( e );
    }

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * Function object version of emit(E&& e)
     */
    void operator()( E&& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( std::move( e ) );
    }

    /*!
     * @brief Adds unit to the queue of outgoing events of the linked event source node
     *
     * Function object version of emit()
     *
     */
    void operator()() const
    {
        static_assert( std::is_same_v<E, unit>, "Can't emit on non unit stream" );
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( unit{} );
    }

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * Stream version of emit(const E& e)
     *
     * @note chaining multiple events in a single statement will not execute them in a single transaction
     */
    const event_source& operator<<( const E& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( e );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * Stream version of emit(E&& e)
     *
     * Specialization of operator<<(const E& e) for rvalue
     *
     * @note chaining multiple events in a single statement will not execute them in a single transaction
     */
    const event_source& operator<<( E&& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( std::move( e ) );
        return *this;
    }

    /*!
     * @brief Returns std compatible iterator for emitting using std algorithms like std::copy()
     */
    auto begin() const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        return iterator{ *this };
    }
};

/*!
 * @brief Iterator for compatibility with std algorithms
 */
template <typename E>
class event_source<E>::iterator final
{
public:
    using iterator_category = std::output_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = E;
    using pointer = value_type*;
    using reference = value_type&;

    /*!
     * @brief Constructor
     */
    explicit iterator( const event_source& parent )
        : m_parent( &parent )
    {}

    // clang-format off
    iterator& operator*()       { return *this; }
    iterator& operator++()      { return *this; }
    iterator  operator++( int ) { return *this; } // NOLINT
    // clang-format on

    /*!
     * @brief Adds e to the queue of outgoing events
     */
    iterator& operator=( const E& e )
    {
        m_parent->emit_event( e );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     */
    iterator& operator=( E&& e )
    {
        m_parent->emit_event( std::move( e ) );
        return *this;
    }

private:
    const event_source* m_parent;
};

/*!
 * @brief Interface for events<S> that allows construction and assigment only for Owner class
 * 
 * member_events is intended to be used as type for public members, so everybody can freely
 * access its public interface while being restricted from reassignment that should be allowed
 * only for the Owner class
 */
template <typename Owner, typename E>
class member_events : public events<E>
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_events
     */
    member_events() = default;

    /*!
     * @brief Copy construct from the given events
     */
    member_events( const events<E>& src ) // NOLINT(google-explicit-constructor)
        : member_events::events( src )
    {}

    /*!
     * @brief Move construct from the given events
     */
    member_events( events<E>&& src ) noexcept // NOLINT(google-explicit-constructor)
        : member_events::events( std::move( src ) )
    {}
};

/*!
 * @brief Interface for event_source<S> that allows construction and assigment only for Owner class
 * 
 * member_event_source is intended to be used as type for public members, so everybody can freely
 * access its public interface while being restricted from reassignment that should be allowed
 * only for the Owner class
 */
template <typename Owner, typename E>
class member_event_source : public event_source<E>
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_event_source
     */
    member_event_source() = default;

    /*!
     * @brief Copy construct from the given event_source
     */
    member_event_source( const event_source<E>& src ) // NOLINT(google-explicit-constructor)
        : member_event_source::event_source( src )
    {}

    /*!
     * @brief Move construct from the given event_source
     */
    member_event_source( event_source<E>&& src ) noexcept // NOLINT(google-explicit-constructor)
        : member_event_source::event_source( std::move( src ) )
    {}
};

/// Base class to setup aliases to member events classes with specific owner class
template <class Owner>
class member_events_user
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_events_user
     */
    member_events_user() = default;

    template <class E>
    using member_events = member_events<Owner, E>;

    template <class E>
    using member_event_source = member_event_source<Owner, E>;
};

/// Macro to setup aliases to member events classes with specific owner class
#define UREACT_USE_MEMBER_EVENTS( Owner )                                                          \
    template <class E>                                                                             \
    using member_events = ::ureact::member_events<Owner, E>;                                       \
    template <class E>                                                                             \
    using member_event_source = ::ureact::member_event_source<Owner, E>

namespace detail
{

template <typename E>
UREACT_WARN_UNUSED_RESULT auto make_event_source( context& context )
{
    return event_source<E>{ std::make_shared<event_source_node<E>>( context ) };
}

} // namespace detail

/*!
 * @brief Create a new event source node and links it to the returned event_source instance
 *
 *  Event value type E has to be specified explicitly. It would be unit if it is omitted.
 */
template <typename E = unit>
UREACT_WARN_UNUSED_RESULT auto make_source( context& context ) -> event_source<E>
{
    assert(
        !_get_internals( context ).get_graph().is_locked() && "Can't make source from callback" );
    return detail::make_event_source<E>( context );
}

/*!
 * @brief Create a new events node and links it to the returned events instance
 *
 *  Event value type E has to be specified explicitly. It would be unit if it is omitted.
 *
 *  Returned value doesn't have input interface and can be used a placeholder where events is required.
 *  Currently it's just cosmetic function that can be expressed as events<E>{ make_source<E>(context) }
 *  but it can be optimized in future.
 */
template <typename E = unit>
UREACT_WARN_UNUSED_RESULT auto make_never( context& context ) -> events<E>
{
    assert(
        !_get_internals( context ).get_graph().is_locked() && "Can't make never from callback" );
    return detail::make_event_source<E>( context );
}

UREACT_END_NAMESPACE

#endif //UREACT_EVENTS_HPP
