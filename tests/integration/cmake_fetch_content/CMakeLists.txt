cmake_minimum_required(VERSION 3.14)
project(cmake_test LANGUAGES CXX)

# =====================================

# gersemi: off
include(FetchContent)

FetchContent_Declare(
    ureact
    GIT_REPOSITORY https://github.com/YarikTH/ureact.git
    GIT_TAG main
)

FetchContent_MakeAvailable(ureact)
# gersemi: on

# =====================================

# Add test target using ureact
add_executable(cmake_test test.cpp)

target_link_libraries(cmake_test PRIVATE ureact::ureact)
