//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_FLATTEN_HPP
#define UREACT_FLATTEN_HPP

#include "ureact.hpp"

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename OuterS, typename InnerS>
class signal_flatten_node final : public signal_node<InnerS>
{
public:
    signal_flatten_node( context& context,
        std::shared_ptr<signal_node<OuterS>> outer,
        const std::shared_ptr<signal_node<InnerS>>& inner )
        : signal_flatten_node::signal_node( context, inner->value_ref() )
        , m_outer( std::move( outer ) )
        , m_inner( inner )
    {
        this->get_graph().on_node_attach( *this, *m_outer );
        this->get_graph().on_node_attach( *this, *m_inner );
    }

    ~signal_flatten_node() override
    {
        this->get_graph().on_node_detach( *this, *m_inner );
        this->get_graph().on_node_detach( *this, *m_outer );
    }

    void tick( turn_type& ) override
    {
        const auto& new_inner = m_outer->value_ref().get_node();

        if( new_inner != m_inner )
        {
            // Topology has been changed
            auto old_inner = m_inner;
            m_inner = new_inner;

            this->get_graph().on_dynamic_node_detach( *this, *old_inner );
            this->get_graph().on_dynamic_node_attach( *this, *new_inner );

            return;
        }

        if( !equals( this->m_value, m_inner->value_ref() ) )
        {
            this->m_value = m_inner->value_ref();
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    std::shared_ptr<signal_node<OuterS>> m_outer;
    std::shared_ptr<signal_node<InnerS>> m_inner;
};

template <typename OuterS, typename InnerE>
class event_flatten_node final : public event_stream_node<InnerE>
{
public:
    event_flatten_node( context& context,
        const std::shared_ptr<signal_node<OuterS>>& outer,
        const std::shared_ptr<event_stream_node<InnerE>>& inner )
        : event_flatten_node::event_stream_node( context )
        , m_outer( outer )
        , m_inner( inner )
    {
        this->get_graph().on_node_attach( *this, *m_outer );
        this->get_graph().on_node_attach( *this, *m_inner );
    }

    ~event_flatten_node() override
    {
        this->get_graph().on_node_detach( *this, *m_outer );
        this->get_graph().on_node_detach( *this, *m_inner );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );
        m_inner->set_current_turn( turn );

        auto new_inner = m_outer->value_ref().get_node();

        if( new_inner != m_inner )
        {
            new_inner->set_current_turn( turn );

            // Topology has been changed
            auto m_old_inner = m_inner;
            m_inner = new_inner;

            this->get_graph().on_dynamic_node_detach( *this, *m_old_inner );
            this->get_graph().on_dynamic_node_attach( *this, *new_inner );

            return;
        }

        this->m_events.insert(
            this->m_events.end(), m_inner->events().begin(), m_inner->events().end() );

        if( this->m_events.size() > 0 )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    std::shared_ptr<signal_node<OuterS>> m_outer;
    std::shared_ptr<event_stream_node<InnerE>> m_inner;
};

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

template <typename T>
using decay_input_t = typename decay_input<T>::type;

} // namespace detail

/*!
 * @brief Create a new signal by flattening a signal of a signal
 */
template <typename InnerS>
UREACT_WARN_UNUSED_RESULT auto flatten( const signal<signal<InnerS>>& outer ) -> signal<InnerS>
{
    context& context = outer.get_context();
    return signal<InnerS>{ std::make_shared<detail::signal_flatten_node<signal<InnerS>, InnerS>>(
        context, outer.get_node(), outer.get().get_node() ) };
}

/*!
 * @brief Create a new event stream by flattening a signal of an event stream
 */
template <typename InnerE>
UREACT_WARN_UNUSED_RESULT auto flatten( const signal<events<InnerE>>& outer ) -> events<InnerE>
{
    context& context = outer.get_context();
    return events<InnerE>{ std::make_shared<detail::event_flatten_node<events<InnerE>, InnerE>>(
        context, outer.get_node(), outer.get().get_node() ) };
}

/*!
 * @brief Utility to flatten public signal attribute of class pointed be reference
 *
 *  For example we have a class Foo with a public signal bar: struct Foo{ signal<int> bar; };
 *  Also, we have signal that points to this class by reference: signal<Foo&> bar
 *  This utility receives a signal reference bar and attribute pointer &Foo::bar and flattens it to signal<int> foobar
 *
 *  @sa reactive_ptr does the same, but receives signal<Foo*>
 */
template <typename S, typename R, typename DecayedR = detail::decay_input_t<R>>
UREACT_WARN_UNUSED_RESULT auto reactive_ref(
    const signal<std::reference_wrapper<S>>& outer, R S::*attribute )
{
    return flatten( lift(
        outer, [attribute]( const S& s ) { return static_cast<DecayedR>( s.*attribute ); } ) );
}

/*!
 * @brief Utility to flatten public signal attribute of class pointed be pointer
 *
 *  For example we have a class Foo with a public signal bar: struct Foo{ signal<int> bar; };
 *  Also, we have signal that points to this class by pointer: signal<Foo*> bar
 *  This utility receives a signal reference bar and attribute pointer &Foo::bar and flattens it to signal<int> foobar
 *
 *  @sa reactive_ref does the same, but receives signal<Foo&>
 */
template <typename S, typename R, typename DecayedR = detail::decay_input_t<R>>
UREACT_WARN_UNUSED_RESULT auto reactive_ptr( const signal<S*>& outer, R S::*attribute )
{
    return flatten( lift(
        outer, [attribute]( const S* s ) { return static_cast<DecayedR>( s->*attribute ); } ) );
}

UREACT_END_NAMESPACE

#endif // UREACT_FLATTEN_HPP