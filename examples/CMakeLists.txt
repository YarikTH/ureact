
function( ureact_add_example name )
    add_executable(example_${name} src/${name}.cpp)
    target_link_libraries(example_${name} ureact::ureact)
endfunction()


ureact_add_example( observers )
ureact_add_example( signals )

ureact_add_example( reactive_class_members )
ureact_add_example( signals_of_references )
ureact_add_example( dynamic_signal_references )
