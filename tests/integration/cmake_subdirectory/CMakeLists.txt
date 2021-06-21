cmake_minimum_required(VERSION 3.8)
project(cmake_test LANGUAGES CXX)

# =====================================

set(PATH_TO_UREACT ../../..)

add_subdirectory(${PATH_TO_UREACT} thirdparty/ureact)

# =====================================

# Add test target using ureact
add_executable(cmake_test test.cpp)

target_link_libraries(cmake_test PRIVATE ureact::ureact)
