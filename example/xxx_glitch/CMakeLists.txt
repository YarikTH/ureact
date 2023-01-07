if(TARGET Boost::boost)
    add_executable(xxx_glitch_example_boost glitch_example_boost.cpp)

    target_link_libraries(xxx_glitch_example_boost PRIVATE Boost::boost)

    target_compile_options(
        xxx_glitch_example_boost
        PRIVATE ${UREACT_WARNING_OPTION}
    )
endif()

if(ON)
    add_executable(
        xxx_glitch_free_example_ureact
        glitch_free_example_ureact.cpp
    )

    target_link_libraries(xxx_glitch_free_example_ureact PRIVATE ureact::ureact)

    target_compile_options(
        xxx_glitch_free_example_ureact
        PRIVATE ${UREACT_WARNING_OPTION}
    )
endif()
