add_executable(ureact_playground)

target_sources(ureact_playground PRIVATE main.cpp)

target_link_libraries(ureact_playground PRIVATE ureact::ureact)

target_compile_options(ureact_playground PRIVATE ${UREACT_WARNING_OPTION})
