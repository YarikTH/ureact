//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_EVENTS_HPP
#define UREACT_EVENTS_HPP

#include <cassert>
#include <type_traits>

#include <ureact/context.hpp>
#include <ureact/detail/node.hpp>
#include <ureact/utility/event_range.hpp> // event ranges often needed along with events.hpp header
#include <ureact/utility/unit.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename E>
class event_stream_node : public node
{
public:
    using event_value_list = std::vector<E>;

    explicit event_stream_node( const context& context )
        : node( context )
    {}

    event_value_list& get_events()
    {
        return m_events;
    }

    const event_value_list& get_events() const
    {
        return m_events;
    }

    void finalize() override
    {
        m_events.clear();
    }

private:
    event_value_list m_events;
};

template <typename E>
class event_source_node final : public event_stream_node<E>
{
public:
    explicit event_source_node( const context& context )
        : event_source_node::event_stream_node( context )
    {}

    ~event_source_node() override = default;

    template <typename V>
    void emit_value( V&& v )
    {
        this->get_events().push_back( std::forward<V>( v ) );
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        return !this->get_events().empty() ? update_result::changed : update_result::unchanged;
    }
};

template <typename E>
class events_internals
{
public:
    events_internals() = default;

    template <typename Node>
    explicit events_internals( std::shared_ptr<Node>&& node )
        : m_node( std::move( node ) )
    {}

    UREACT_WARN_UNUSED_RESULT std::shared_ptr<event_stream_node<E>>& get_node_ptr()
    {
        return m_node;
    }

    UREACT_WARN_UNUSED_RESULT const std::shared_ptr<event_stream_node<E>>& get_node_ptr() const
    {
        return m_node;
    }

    UREACT_WARN_UNUSED_RESULT node_id get_node_id() const
    {
        return m_node->get_node_id();
    }

    std::vector<E>& get_events()
    {
        return m_node->get_events();
    }

    const std::vector<E>& get_events() const
    {
        return m_node->get_events();
    }

private:
    UREACT_WARN_UNUSED_RESULT auto get_event_source_node() const -> event_source_node<E>*
    {
        return dynamic_cast<event_source_node<E>*>( this->m_node.get() );
    }

protected:
    UREACT_WARN_UNUSED_RESULT react_graph& get_graph() const
    {
        assert( m_node != nullptr && "Should be attached to a node" );
        return get_internals( m_node->get_context() ).get_graph();
    }

    template <typename T>
    void emit_event( T&& e ) const
    {
        react_graph& graph_ref = get_graph();
        assert( !graph_ref.is_locked() && "Can't emit event from callback" );

        event_source_node<E>* node_ptr = get_event_source_node();
        node_ptr->emit_value( std::forward<T>( e ) );
        graph_ref.push_input( node_ptr->get_node_id() );
    }

    std::shared_ptr<event_stream_node<E>> m_node;
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
class events : protected detail::events_internals<E>
{
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
     * @brief Equally compare with other @ref events
     * 
     * Semantic equivalent of operator==
     */
    UREACT_WARN_UNUSED_RESULT bool equal_to( const events& other ) const
    {
        return this->get_node_ptr() == other.get_node_ptr();
    }

    /*!
     * @brief Return internals. Not intended to use in user code
     */
    UREACT_WARN_UNUSED_RESULT friend detail::events_internals<E>& get_internals( events<E>& e )
    {
        return e;
    }

    /*!
     * @brief Return internals. Not intended to use in user code
     */
    UREACT_WARN_UNUSED_RESULT friend const detail::events_internals<E>& get_internals(
        const events<E>& e )
    {
        return e;
    }

protected:
    using Node = detail::event_stream_node<E>;

    /*!
     * @brief Construct from the given node
     */
    explicit events( std::shared_ptr<Node>&& node )
        : events::events_internals( std::move( node ) )
    {}

    template <typename Ret, typename Node, typename... Args>
    friend Ret detail::create_wrapped_node( Args&&... args );

private:
    /*!
     * @brief has_changed overload for @ref events
     */
    UREACT_WARN_UNUSED_RESULT friend constexpr bool has_changed(
        const events& lhs, const events& rhs ) noexcept
    {
        return !lhs.equal_to( rhs );
    }
};

/*!
 * @brief @ref events that support imperative input
 *
 *  An event source extends the immutable @ref events interface with functions that support imperative input.
 */
template <typename E = unit>
class event_source : public events<E>
{
public:
    class iterator;

    /*!
     * @brief Default construct @ref event_source
     *
     * Default constructed @ref event_source is not attached to node, so it is not valid
     */
    event_source() = default;

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

protected:
    using Node = detail::event_source_node<E>;

    /*!
     * @brief Construct from the given node
     */
    explicit event_source( std::shared_ptr<Node>&& node )
        : event_source::events( std::move( node ) )
    {}

    template <typename Ret, typename Node, typename... Args>
    friend Ret detail::create_wrapped_node( Args&&... args );
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

    UREACT_MAKE_COPYABLE( member_events );
    UREACT_MAKE_MOVABLE( member_events );

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

    UREACT_MAKE_COPYABLE( member_event_source );
    UREACT_MAKE_MOVABLE( member_event_source );

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

/// Macro to setup aliases to member events classes with specific owner class
#define UREACT_USE_MEMBER_EVENTS( Owner )                                                          \
    template <class E>                                                                             \
    using member_events = ::ureact::member_events<Owner, E>;                                       \
    template <class E>                                                                             \
    using member_event_source = ::ureact::member_event_source<Owner, E>

/// Base class to setup aliases to member events classes with specific owner class
template <class Owner>
class member_events_user
{
    friend Owner;

    member_events_user() = default;

    UREACT_USE_MEMBER_EVENTS( Owner );
};


/*!
 * @brief Create a new event source node and links it to the returned event_source instance
 *
 *  Event value type E has to be specified explicitly. It would be unit if it is omitted.
 */
template <typename E = unit>
UREACT_WARN_UNUSED_RESULT auto make_source( const context& context ) -> event_source<E>
{
    assert(
        !get_internals( context ).get_graph().is_locked() && "Can't make source from callback" );
    return detail::create_wrapped_node<event_source<E>, detail::event_source_node<E>>( context );
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
UREACT_WARN_UNUSED_RESULT auto make_never( const context& context ) -> events<E>
{
    assert( !get_internals( context ).get_graph().is_locked() && "Can't make never from callback" );
    return detail::create_wrapped_node<events<E>, detail::event_source_node<E>>( context );
}

namespace default_context
{

/*!
 * @brief Create a new event source node and links it to the returned event_source instance
 */
template <typename E = unit>
UREACT_WARN_UNUSED_RESULT auto make_source() -> event_source<E>
{
    return make_source<E>( default_context::get() );
}

/*!
 * @brief Create a new events node and links it to the returned events instance
 */
template <typename E = unit>
UREACT_WARN_UNUSED_RESULT auto make_never() -> events<E>
{
    return make_never<E>( default_context::get() );
}

} // namespace default_context

UREACT_END_NAMESPACE

#endif //UREACT_EVENTS_HPP
