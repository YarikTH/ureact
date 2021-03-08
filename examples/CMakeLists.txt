
function( ureact_add_example name )
    add_executable(example_${name} src/${name}.cpp)
    target_link_libraries(example_${name} ureact::ureact)
endfunction()


ureact_add_example( hello_world_operators )
ureact_add_example( hello_world_make_signal )
ureact_add_example( modifying_signal_values_in_place )
ureact_add_example( changing_multiple_inputs )
ureact_add_example( reacting_to_value_changes )
ureact_add_example( complex_signals_v1 )
ureact_add_example( complex_signals_v2 )
ureact_add_example( complex_signals_v3 )

ureact_add_example( creating_subject_bound_observers_v1 )
ureact_add_example( creating_subject_bound_observers_v2 )
ureact_add_example( detaching_observers_manually )
ureact_add_example( detaching_observers_by_themselves )
ureact_add_example( using_scoped_observers )

ureact_add_example( reactive_class_members )
ureact_add_example( signals_of_references )
ureact_add_example( dynamic_signal_references )
