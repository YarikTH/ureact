add_subdirectory(thirdparty/doctest)

add_executable(ureact_test)

target_sources(
    ureact_test
    PRIVATE
        src/main.cpp
        src/examples/signal_examples.cpp
        src/details/move_test.cpp
        src/details/observer_test.cpp
        src/details/signal_test.cpp
        src/details/operators_test.cpp
        src/details/dynamic_signals_test.cpp
)

target_link_libraries(ureact_test PRIVATE ureact::ureact ureact::doctest)

target_compile_options(ureact_test PRIVATE ${UREACT_WARNING_OPTION})

add_test(NAME ureact_test COMMAND ureact_test)
