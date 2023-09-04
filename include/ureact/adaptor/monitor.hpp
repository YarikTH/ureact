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

#include <ureact/core/adaptor.hpp>
#include <ureact/events.hpp>
#include <ureact/signal.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
class monitor_node final : public event_stream_node<S>
{
public:
    monitor_node( const context& context, const signal<S>& target )
        : monitor_node::event_stream_node( context )
        , m_target( target )
    {
        this->attach_to( m_target );
    }

    ~monitor_node() override
    {
        this->detach_from_all();
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        this->get_events().push_back( get_internals( m_target ).get_value() );

        return update_result::changed;
    }

private:
    signal<S> m_target;
};

struct MonitorClosure : core::adaptor_closure
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
        const context& context = target.get_context();
        return detail::create_wrapped_node<events<S>, monitor_node<S>>( context, target );
    }
};

} // namespace detail


inline constexpr detail::MonitorClosure monitor;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_MONITOR_HPP
