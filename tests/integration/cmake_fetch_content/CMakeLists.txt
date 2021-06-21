cmake_minimum_required(VERSION 3.11)
project(cmake_test LANGUAGES CXX)

# =====================================

# gersemi: off
include(FetchContent)

FetchContent_Declare(
    ureact
    GIT_REPOSITORY https://github.com/YarikTH/ureact.git
    GIT_TAG main
)

FetchContent_GetProperties(ureact)
if(NOT ureact_POPULATED)
    FetchContent_Populate(ureact)
    add_subdirectory(${ureact_SOURCE_DIR} ${ureact_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
# gersemi: on

# =====================================

# Add test target using ureact
add_executable(cmake_test test.cpp)

target_link_libraries(cmake_test PRIVATE ureact::ureact)
