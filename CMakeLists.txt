#
#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.8)

################################################################################
## UREACT
################################################################################

# Get version from ureact.hpp
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/include/ureact/ureact.hpp ureact_hpp)
if(NOT ureact_hpp MATCHES "UREACT_VERSION_STR \"([0-9]+).([0-9]+).([0-9]+)\"")
    message(FATAL_ERROR "Cannot get UREACT_VERSION from ureact.hpp.")
endif()
# Use math to skip leading zeros if any.
math(EXPR UREACT_VERSION_MAJOR ${CMAKE_MATCH_1})
math(EXPR UREACT_VERSION_MINOR ${CMAKE_MATCH_2})
math(EXPR UREACT_VERSION_PATCH ${CMAKE_MATCH_3})
set(UREACT_VERSION
    ${UREACT_VERSION_MAJOR}.${UREACT_VERSION_MINOR}.${UREACT_VERSION_PATCH}
)

project(ureact VERSION ${UREACT_VERSION} LANGUAGES CXX)

# Determine if ureact is built as a subproject (using add_subdirectory)
# or if it is the master project.
if(NOT DEFINED UREACT_MASTER_PROJECT)
    set(UREACT_MASTER_PROJECT OFF)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(UREACT_MASTER_PROJECT ON)
        message(STATUS "CMake version: ${CMAKE_VERSION}")
    endif()
endif()

# Options that control generation of various targets.
option(UREACT_PEDANTIC "Enable extra warnings and expensive tests." OFF)
option(
    UREACT_WERROR
    "Halt the compilation with an error on compiler warnings."
    OFF
)
option(UREACT_TEST "Generate the test target." ${UREACT_MASTER_PROJECT})
option(UREACT_INSTALL "Generate install target." ${UREACT_MASTER_PROJECT})
option(UREACT_СTEST "Generate the CTest targets for CI." n)

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
add_library(${PROJECT_NAME} INTERFACE)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(
    ${PROJECT_NAME}
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
if(MSVC)
    # See https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
    target_compile_options(${PROJECT_NAME} INTERFACE "/Zc:__cplusplus")
endif()

################################################################################
## TESTS/EXAMPLES/HELPERS
################################################################################

if(UREACT_TEST)
    if(UREACT_СTEST)
        include(CTest)
    else()
        enable_testing()
    endif()
    add_subdirectory(tests)
endif()

################################################################################
## PACKAGE SUPPORT
################################################################################

# cmake code for install is based on https://github.com/onqtam/doctest/blob/master/CMakeLists.txt

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    include(GNUInstallDirs)
    set(include_install_dir ${CMAKE_INSTALL_INCLUDEDIR})
    set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
else()
    set(include_install_dir "include")
    set(config_install_dir "lib/cmake/${PROJECT_NAME}")
endif()

set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

include(CMakePackageConfigHelpers)

# CMake automatically adds an architecture compatibility check to make sure
# 32 and 64 bit code is not accidentally mixed. For a header-only library this
# is not required. The check can be disabled by temporarily unsetting
# CMAKE_SIZEOF_VOID_P. In CMake 3.14 and later this can be achieved more cleanly
# with write_basic_package_version_file(ARCH_INDEPENDENT).
# TODO: Use this once a newer CMake can be required.
set(UREACT_SIZEOF_VOID_P ${CMAKE_SIZEOF_VOID_P})
unset(CMAKE_SIZEOF_VOID_P)
write_basic_package_version_file(
    "${version_config}"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
set(CMAKE_SIZEOF_VOID_P ${UREACT_SIZEOF_VOID_P})

configure_file("support/cmake/Config.cmake.in" "${project_config}" @ONLY)

if(UREACT_INSTALL)
    install(
        TARGETS ${PROJECT_NAME}
        EXPORT "${targets_export_name}"
        INCLUDES DESTINATION "${include_install_dir}"
    )

    install(
        FILES "include/ureact/ureact.hpp"
        DESTINATION "${include_install_dir}/ureact"
    )

    install(
        FILES "${project_config}" "${version_config}"
        DESTINATION "${config_install_dir}"
    )

    install(
        EXPORT "${targets_export_name}"
        NAMESPACE "${namespace}"
        DESTINATION "${config_install_dir}"
    )
endif()
