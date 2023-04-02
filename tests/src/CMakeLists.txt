#
#         Copyright (C) 2020-2023 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
add_executable(ureact_test main.cpp)

target_sources(
    ureact_test
    PRIVATE
        adaptor.cpp
        cast.cpp
        changed.cpp
        collect.cpp
        context.cpp
        count.cpp
        default_context.cpp
        detail.cpp
        elements.cpp
        event_emitter.cpp
        event_range.cpp
        events.cpp
        filter.cpp
        flatten.cpp
        fold.cpp
        hold.cpp
        join.cpp
        lift.cpp
        merge.cpp
        monitor.cpp
        monitor_change.cpp
        observer.cpp
        process.cpp
        propagation.cpp
        pulse.cpp
        scoped_observer.cpp
        signal.cpp
        snapshot.cpp
        stride.cpp
        take_drop.cpp
        take_drop_while.cpp
        tap.cpp
        transaction.cpp
        transform.cpp
        unify.cpp
        unique.cpp
        zip.cpp
)

target_link_libraries(ureact_test PRIVATE ureact::ureact ureact::doctest)

target_compile_options(ureact_test PRIVATE ${UREACT_WARNING_OPTION})

add_test(NAME ureact_test COMMAND ureact_test)
