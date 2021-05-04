function(ureact_add_example name)
    add_executable(example_${name} src/${name}.cpp)
    target_link_libraries(example_${name} ureact::ureact)
    target_compile_options(example_${name} PRIVATE ${UREACT_WARNING_OPTION})
endfunction()

ureact_add_example( reactive_class_members )
ureact_add_example( signals_of_references )
ureact_add_example( dynamic_signal_references )
