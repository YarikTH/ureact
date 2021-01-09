### example_composition
add_executable(example_composition src/composition.cpp)
target_link_libraries(example_composition ureact::ureact)

### example_observers
add_executable(example_observers src/observers.cpp)
target_link_libraries(example_observers ureact::ureact)

### example_signals
add_executable(example_signals src/signals.cpp)
target_link_libraries(example_signals ureact::ureact)
