cmake_minimum_required(VERSION 3.1)

project(ureact VERSION 0.0.1 LANGUAGES CXX)

# Determine if ureact is built as a subproject (using add_subdirectory)
# or if it is the master project.
if(NOT DEFINED UREACT_MASTER_PROJECT)
    set(UREACT_MASTER_PROJECT OFF)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(UREACT_MASTER_PROJECT ON)
        message(STATUS "CMake version: ${CMAKE_VERSION}")
    endif()
endif()

option(UREACT_PEDANTIC "Enable extra warnings and expensive tests." OFF)
option(
    UREACT_WERROR
    "Halt the compilation with an error on compiler warnings."
    OFF
)

# Options that control generation of various targets.
option(UREACT_TEST "Generate the test target." ${UREACT_MASTER_PROJECT})
option(
    UREACT_PLAYGROUND
    "Generate the playground target."
    ${UREACT_MASTER_PROJECT}
)

# Get version from core.h
file(
    READ
        ${CMAKE_CURRENT_SOURCE_DIR}/include/ureact/detail/version.hpp
        version_hpp
)
if(NOT version_hpp MATCHES "UREACT_VERSION ([0-9]+)([0-9][0-9])([0-9][0-9])")
    message(FATAL_ERROR "Cannot get UREACT_VERSION from version_hpp.")
endif()
# Use math to skip leading zeros if any.
math(EXPR UREACT_VERSION_MAJOR ${CMAKE_MATCH_1})
math(EXPR UREACT_VERSION_MINOR ${CMAKE_MATCH_2})
math(EXPR UREACT_VERSION_PATCH ${CMAKE_MATCH_3})
set(UREACT_VERSION
    ${UREACT_VERSION_MAJOR}.${UREACT_VERSION_MINOR}.${UREACT_VERSION_PATCH}
)
message(STATUS "ureact version: ${UREACT_VERSION}")

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/support/cmake")

include(set_warning_flags)

set(UREACT_WARNING_OPTION)

if(UREACT_PEDANTIC)
    set_warning_flags(UREACT_WARNING_OPTION)
endif()

if(UREACT_WERROR)
    set_werror_flag(UREACT_WARNING_OPTION)
endif()
message(STATUS "ureact compiler warning options: \"${UREACT_WARNING_OPTION}\"")

### ureact
add_library(ureact INTERFACE)

add_library(ureact::ureact ALIAS ureact)

target_include_directories(
    ureact
    INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(ureact INTERFACE cxx_std_11)

### tests/
if(UREACT_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()
