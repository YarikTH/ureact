add_subdirectory(thirdparty/doctest)
add_subdirectory(thirdparty/ApprovalTests.cpp)

add_executable(ureact_test
	src/main.cpp
	src/move_test.cpp
	src/observer_test.cpp
	src/signal_test.cpp)

target_link_libraries(ureact_test
					  ureact::ureact
					  ureact::doctest
					  ureact::ApprovalTests)
