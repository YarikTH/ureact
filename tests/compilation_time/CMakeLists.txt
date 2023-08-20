#
#         Copyright (C) 2020-2023 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
add_executable(ureact_compilation_time_benchmarks EXCLUDE_FROM_ALL)

target_sources(
    ureact_compilation_time_benchmarks
    PRIVATE #
        compiler_args.cpp
        std_headers_include_time.cpp
        transform_compilation.cpp
)

target_link_libraries(
    ureact_compilation_time_benchmarks
    PRIVATE #
        ureact::ureact
        Catch2::Catch2WithMain
        nanobench::nanobench
)

target_compile_options(
    ureact_compilation_time_benchmarks
    PRIVATE #
        ${UREACT_WARNING_OPTION}
)

get_filename_component(
    UREACT_INCLUDE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
    ABSOLUTE
)
get_filename_component(
    TEST_SAMPLES_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/test_samples"
    ABSOLUTE
)

target_compile_definitions(
    ureact_compilation_time_benchmarks
    PRIVATE #
        UREACT_INCLUDE_PATH="${UREACT_INCLUDE_PATH}"
        TEST_SAMPLES_PATH="${TEST_SAMPLES_PATH}"
)
