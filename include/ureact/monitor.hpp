//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_MONITOR_HPP_
#define UREACT_MONITOR_HPP_

#include "closure.hpp"
#include "ureact.hpp"

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename E>
class monitor_node final : public event_stream_node<E>
{
public:
    monitor_node( context& context, const std::shared_ptr<signal_node<E>>& target )
        : monitor_node::event_stream_node( context )
        , m_target( target )
    {
        this->get_graph().on_node_attach( *this, *m_target );
    }

    ~monitor_node() override
    {
        this->get_graph().on_node_detach( *this, *m_target );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );

        this->m_events.push_back( m_target->value_ref() );

        this->pulse_if_has_events();
    }

private:
    const std::shared_ptr<signal_node<E>> m_target;
};

} // namespace detail

/*!
 * @brief Emits value changes of signal as events
 *
 *  When target changes, emit the new value 'e = target.get()'.
 */
template <typename S>
UREACT_WARN_UNUSED_RESULT auto monitor( const signal<S>& target ) -> events<S>
{
    context& context = target.get_context();
    return events<S>( std::make_shared<detail::monitor_node<S>>( context, target.get_node() ) );
}

/*!
 * @brief Curried version of monitor(const signal<S>& target)
 */
UREACT_WARN_UNUSED_RESULT inline auto monitor()
{
    return closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_signal_v<std::decay_t<arg_t>>, "Signal type is required" );
        return monitor( std::forward<arg_t>( source ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_MONITOR_HPP_
