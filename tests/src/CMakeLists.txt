#
#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
add_executable(
    ureact_test
    main.cpp
    examples/signal_examples.cpp
    examples/complex_signal_examples.cpp
    examples/composition_examples.cpp
    examples/observer_examples.cpp
    details/observer_test.cpp
    details/signal_test.cpp
    details/operators_test.cpp
    details/dynamic_signals_test.cpp
    details/ObserverTest.cpp
    details/EventStreamTest.cpp
    details/SignalTest.cpp
    details/OperationsTest.cpp
)

target_include_directories(ureact_test PRIVATE include)

target_link_libraries(ureact_test PRIVATE ureact::ureact ureact::doctest)

target_compile_options(ureact_test PRIVATE ${UREACT_WARNING_OPTION})

add_test(NAME ureact_test COMMAND ureact_test)
