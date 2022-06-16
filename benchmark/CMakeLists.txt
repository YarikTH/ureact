#
#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.14)

# gersemi: off
include(FetchContent)

FetchContent_Declare(
    nanobench
    GIT_REPOSITORY "https://github.com/martinus/nanobench.git"
    GIT_TAG "v4.3.7"
)

FetchContent_GetProperties(nanobench)
if(NOT nanobench_POPULATED)
    FetchContent_Populate(nanobench)
    add_subdirectory(${nanobench_SOURCE_DIR} ${nanobench_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
# gersemi: on

function(add_benchmark TARGET_NAME SRC)
    add_executable(${TARGET_NAME} ${SRC})

    target_link_libraries(
        ${TARGET_NAME}
        PRIVATE #
            ureact::ureact
            nanobench::nanobench
    )

    target_compile_options(${TARGET_NAME} PRIVATE ${UREACT_WARNING_OPTION})
endfunction()

# benchmarks
add_benchmark(benchmark_signal_composing src/signal_composing.cpp)
add_benchmark(benchmark_hold src/hold.cpp)
add_benchmark(benchmark_snapshot src/snapshot.cpp)
add_benchmark(benchmark_pulse src/pulse.cpp)
