#
#         Copyright (C) 2020-2023 Yaroslav Krylov.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
add_executable(ureact_test)

target_sources(
    ureact_test
    PRIVATE
        adaptor/adjacent.cpp
        adaptor/adjacent_filter.cpp
        adaptor/adjacent_transform.cpp
        adaptor/cast.cpp
        adaptor/changed.cpp
        adaptor/changed_to.cpp
        adaptor/collect.cpp
        adaptor/count.cpp
        adaptor/drop.cpp
        adaptor/drop_while.cpp
        adaptor/elements.cpp
        adaptor/enumerate.cpp
        adaptor/filter.cpp
        adaptor/flatten.cpp
        adaptor/fold.cpp
        adaptor/happened.cpp
        adaptor/hold.cpp
        adaptor/join.cpp
        adaptor/join_with.cpp
        adaptor/keys.cpp
        adaptor/lift.cpp
        adaptor/merge.cpp
        adaptor/monitor.cpp
        adaptor/monitor_change.cpp
        adaptor/observe.cpp
        adaptor/once.cpp
        adaptor/pairwise.cpp
        adaptor/pairwise_filter.cpp
        adaptor/pairwise_transform.cpp
        adaptor/process.cpp
        adaptor/pulse.cpp
        adaptor/reactive_ref.cpp
        adaptor/slice.cpp
        adaptor/snapshot.cpp
        adaptor/stride.cpp
        adaptor/take.cpp
        adaptor/take_while.cpp
        adaptor/tap.cpp
        adaptor/transform.cpp
        adaptor/unify.cpp
        adaptor/unique.cpp
        adaptor/values.cpp
        adaptor/zip.cpp
        adaptor/zip_transform.cpp
        feature/adaptor.cpp
        feature/default_context.cpp
        feature/has_changed.cpp
        feature/propagation.cpp
        feature/reactive_members.cpp
        feature/shared_ptr_semantics.cpp
        benchmark_signal_composing.cpp
        context.cpp
        detail.cpp
        event_emitter.cpp
        event_range.cpp
        events.cpp
        observer.cpp
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
