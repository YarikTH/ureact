#
#         Copyright (C) 2020-2023 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
add_executable(ureact_test)

target_sources(
    ureact_test
    PRIVATE
        adaptor/cast.cpp
        adaptor/changed.cpp
        adaptor/collect.cpp
        adaptor/count.cpp
        adaptor/elements.cpp
        adaptor/filter.cpp
        adaptor/flatten.cpp
        adaptor/fold.cpp
        adaptor/happened.cpp
        adaptor/hold.cpp
        adaptor/join.cpp
        adaptor/lift.cpp
        adaptor/merge.cpp
        adaptor/monitor.cpp
        adaptor/monitor_change.cpp
        adaptor/process.cpp
        adaptor/pulse.cpp
        adaptor/snapshot.cpp
        adaptor/stride.cpp
        adaptor/take_drop.cpp
        adaptor/take_drop_while.cpp
        adaptor/tap.cpp
        adaptor/transform.cpp
        adaptor/unify.cpp
        adaptor/unique.cpp
        adaptor/zip.cpp
        adaptor.cpp
        benchmak_signal_composing.cpp
        context.cpp
        default_context.cpp
        detail.cpp
        event_emitter.cpp
        event_range.cpp
        events.cpp
        has_changed.cpp
        observer.cpp
        propagation.cpp
        scoped_observer.cpp
        signal.cpp
        transaction.cpp
)

target_link_libraries(
    ureact_test
    PRIVATE ureact::ureact Catch2::Catch2WithMain nanobench::nanobench
)

target_include_directories(ureact_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(ureact_test PRIVATE ${UREACT_WARNING_OPTION})

add_test(NAME ureact_test COMMAND ureact_test)
