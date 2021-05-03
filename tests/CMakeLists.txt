add_subdirectory(thirdparty/doctest)

add_executable(
    ureact_test
    src/main.cpp
    src/move_test.cpp
    src/observer_test.cpp
    src/signal_test.cpp
    src/operators_test.cpp
    src/dynamic_signals_test.cpp
)

target_link_libraries(ureact_test PRIVATE ureact::ureact ureact::doctest)

target_compile_options(ureact_test PRIVATE ${UREACT_WARNING_OPTION})

add_test(NAME ureact_test COMMAND ureact_test)
