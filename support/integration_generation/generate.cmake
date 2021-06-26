#
#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
set(DEST_DIR ${CMAKE_CURRENT_LIST_DIR}/../../tests/integration)

function(generate_integration_cmake name cmake_min_ver)
    set(CMAKE_MIN_VER ${cmake_min_ver})
    file(READ ${CMAKE_CURRENT_LIST_DIR}/${name}.cmake CMAKE_LISTS_TXT_BODY)
    configure_file(
        ${CMAKE_CURRENT_LIST_DIR}/integration_main.cmake
        ${DEST_DIR}/${name}/CMakeLists.txt
        @ONLY
    )
    set(TEST_CPP ../../../support/integration_generation/test.cpp)
    set(TEST_CPP_LINKED_COPY ${DEST_DIR}/${name}/test.cpp)
    file(CREATE_LINK "${TEST_CPP}" "${TEST_CPP_LINKED_COPY}" SYMBOLIC)
endfunction()

set(MAIN_HEADER ../../../../include/ureact/ureact.hpp)
set(MAIN_HEADER_LINKED_COPY ${DEST_DIR}/cmake_copied_header/ureact/ureact.hpp)
file(CREATE_LINK "${MAIN_HEADER}" "${MAIN_HEADER_LINKED_COPY}" SYMBOLIC)

generate_integration_cmake(cmake_copied_header 3.8)
generate_integration_cmake(cmake_external_project 3.8)
generate_integration_cmake(cmake_fetch_content 3.11)
generate_integration_cmake(cmake_find_package 3.8)
generate_integration_cmake(cmake_subdirectory 3.8)
