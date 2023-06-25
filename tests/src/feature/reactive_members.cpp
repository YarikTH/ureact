//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/events.hpp"
#include "ureact/signal.hpp"

// TODO: add tests for:
// * member_signal<S, User>
// * member_var_signal<S, User>
// * member_signal_user<User>
// * UREACT_USE_MEMBER_SIGNALS(User)
//
// * member_events<E, User>
// * member_event_source<E, User>
// * member_events_user<User>
// * UREACT_USE_MEMBER_EVENTS(User)

// TODO: write type trait test to check that signal<S> public member can be assigned outside while member_signal<S, User> can't be
// see has_push_back_method from ureact/adaptor/collect.hpp as an example
