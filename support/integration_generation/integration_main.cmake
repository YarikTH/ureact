cmake_minimum_required(VERSION @CMAKE_MIN_VER@)
project(cmake_test LANGUAGES CXX)

# =====================================

@CMAKE_LISTS_TXT_BODY@
# =====================================

# Add test target using ureact
add_executable(cmake_test test.cpp)

target_link_libraries(cmake_test PRIVATE ureact::ureact)
