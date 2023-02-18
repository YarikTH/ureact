//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_MONITOR_HPP
#define UREACT_ADAPTOR_MONITOR_HPP

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/base.hpp>
#include <ureact/events.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
class signal_node;

template <typename E>
class monitor_node final : public event_stream_node<E>
{
public:
    monitor_node( context& context, const std::shared_ptr<signal_node<E>>& target )
        : monitor_node::event_stream_node( context )
        , m_target( target )
    {
        this->attach_to( *m_target );
    }

    ~monitor_node() override
    {
        this->detach_from( *m_target );
    }

    UREACT_WARN_UNUSED_RESULT update_result update( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );

        this->m_events.push_back( m_target->value_ref() );

        return this->pulse_if_has_events();
    }

private:
    const std::shared_ptr<signal_node<E>> m_target;
};

struct MonitorClosure : AdaptorClosure
{
    /*!
	 * @brief Emits value changes of signal as events
	 *
	 *  When target changes, emit the new value 'e = target.get()'.
	 */
    template <typename S>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const signal<S>& target ) const
        -> events<S>
    {
        context& context = target.get_context();
        return events<S>( std::make_shared<monitor_node<S>>( context, target.get_node() ) );
    }
};

} // namespace detail


inline constexpr detail::MonitorClosure monitor;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_MONITOR_HPP
