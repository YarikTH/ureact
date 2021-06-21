cmake_minimum_required(VERSION 3.8)
project(cmake_test LANGUAGES CXX)

# =====================================

# Prepare ureact for other targets to use
find_package(ureact REQUIRED)

# =====================================

# Add test target using ureact
add_executable(cmake_test test.cpp)

target_link_libraries(cmake_test PRIVATE ureact::ureact)
