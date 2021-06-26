#
#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
include(CMakeParseArguments)

set(TEMP_DIR "${CMAKE_CURRENT_LIST_DIR}/tmp")
set(UREACT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")

# Based on https://stackoverflow.com/a/37064447
function(make_temp_dir)
    set(oneValueArgs PREFIX PARENT OUTPUT_VARIABLE)
    cmake_parse_arguments(MAKE_TEMP "${options}" "${oneValueArgs}" "" ${ARGN})

    if(NOT DEFINED MAKE_TEMP_PREFIX)
        set(MAKE_TEMP_PREFIX "tmp")
    endif()

    if(NOT DEFINED MAKE_TEMP_PARENT)
        set(MAKE_TEMP_PARENT "$ENV{TMP}")
    endif()

    set(_COUNTER 0)
    while(EXISTS "${MAKE_TEMP_PARENT}/${MAKE_TEMP_PREFIX}${_COUNTER}")
        math(EXPR _COUNTER "${_COUNTER} + 1")
    endwhile()
    set(_NAME "${MAKE_TEMP_PARENT}/${MAKE_TEMP_PREFIX}${_COUNTER}")
    set(${MAKE_TEMP_OUTPUT_VARIABLE} "${_NAME}" PARENT_SCOPE)

    file(MAKE_DIRECTORY "${_NAME}")
endfunction()

# Based on https://stackoverflow.com/a/43073402
# evaluates the specified cmake code.
# WARNING: there is no way to set(<var> <value> PARENT_SCOPE)
# because of the extra function scope defined by eval.
# WARNING: allowing eval can of course be dangerous.
function(eval __eval_code)
    make_temp_dir(PARENT "${TEMP_DIR}" PREFIX "eval_" OUTPUT_VARIABLE EVAL_DIR)

    # one file per execution of cmake (if this file were in memory it would probably be faster...)
    # this is where the temporary eval file will be stored.  it will only be used once per eval
    # and since cmake is not multi-threaded no race conditions should occur.  however, if you start
    # two cmake processes in the same project this could lead to collisions
    set(__eval_temp_file "${EVAL_DIR}/eval_temp.cmake")

    # write the content of temp file and include it directly, this overwrite the
    # eval function you are currently defining (initializer function pattern)
    file(
        WRITE
        "${__eval_temp_file}"
        "
function(eval __eval_code)
  file(WRITE ${__eval_temp_file} \"\${__eval_code}\")
  include(${__eval_temp_file})
endfunction()
  "
    )

    include("${__eval_temp_file}")
    # now eval is defined as what was just written into __eval_temp_file

    # since we are still in first definition we just need to execute eval now
    # (which calls the second definition of eval).
    eval("${__eval_code}")

    file(REMOVE_RECURSE "${EVAL_DIR}")
endfunction()

macro(execute_process_checked NAME)
    execute_process(
        ${ARGN}
        RESULT_VARIABLE _result
        OUTPUT_VARIABLE _output
        ERROR_VARIABLE _error
    )
    if(NOT _result STREQUAL "0")
        set(_message "")
        if(_result)
            set(_message "${_message}\nResult: \"${_result}\"")
        endif()
        if(_output)
            set(_message "${_message}\nOutput: \"${_output}\"")
        endif()
        if(_error)
            set(_message "${_message}\nError:  \"${_error}\"")
        endif()
        message(FATAL_ERROR "${NAME} failed. ${_message}")
    elseif(_error)
        message(
            WARNING
            "There were some problems in ${NAME}. See error output:\n${_error}"
        )
    endif()
endmacro()

function(check_cmake_integration NAME)
    set(oneValueArgs BEFORE PREFIX_PATH SOURCE_DIR)
    cmake_parse_arguments(OPTION "${options}" "${oneValueArgs}" "" ${ARGN})

    message("Checking ${NAME}")

    make_temp_dir(PARENT "${TEMP_DIR}" PREFIX "${NAME}_" OUTPUT_VARIABLE WORK_DIR)
    set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/${OPTION_SOURCE_DIR})

    if(OPTION_BEFORE)
        eval("${OPTION_BEFORE}(\${WORK_DIR})")
    endif()

    if(OPTION_PREFIX_PATH)
        set(CMAKE_CONFIGURATION_OPTIONS
            -DCMAKE_PREFIX_PATH=${WORK_DIR}/${OPTION_PREFIX_PATH}
        )
    endif()

    execute_process_checked("${NAME} CMake configuration"
                             COMMAND ${CMAKE_COMMAND} ${CMAKE_CONFIGURATION_OPTIONS} ${SOURCE_DIR}
                             WORKING_DIRECTORY ${WORK_DIR}
    )

    execute_process_checked("${NAME} CMake build"
                             COMMAND ${CMAKE_COMMAND} --build .
                             WORKING_DIRECTORY ${WORK_DIR}
    )

    execute_process_checked("${NAME} Text execution"
                             COMMAND ./cmake_test
                             WORKING_DIRECTORY ${WORK_DIR}
    )

    file(REMOVE_RECURSE "${WORK_DIR}")
endfunction()

function(install_ureact_locally WORK_DIR)
    set(UREACT_SOURCE_DIR "${UREACT_SOURCE_DIR}")
    set(UREACT_BUILD_DIR "${WORK_DIR}/ureact/build")
    set(UREACT_INSTALL_DIR "${WORK_DIR}/ureact/install")

    file(MAKE_DIRECTORY ${UREACT_BUILD_DIR})

    execute_process_checked("µReact CMake configuration"
                             COMMAND ${CMAKE_COMMAND}
                                     -DUREACT_TEST=n
                                     -DCMAKE_INSTALL_PREFIX=${UREACT_INSTALL_DIR}
                                     ${UREACT_SOURCE_DIR}
                             WORKING_DIRECTORY ${UREACT_BUILD_DIR}
    )

    execute_process_checked("µReact CMake build"
                             COMMAND ${CMAKE_COMMAND} --build . --target install
                             WORKING_DIRECTORY ${UREACT_BUILD_DIR}
    )
endfunction()

check_cmake_integration(cmake_copied_header SOURCE_DIR cmake_copied_header)
check_cmake_integration(cmake_find_package SOURCE_DIR cmake_find_package BEFORE install_ureact_locally PREFIX_PATH ureact/install)
check_cmake_integration(cmake_subdirectory SOURCE_DIR cmake_subdirectory)
check_cmake_integration(cmake_external_project SOURCE_DIR cmake_external_project)
if(CMAKE_VERSION VERSION_EQUAL "3.11" OR CMAKE_VERSION VERSION_GREATER "3.11")
    check_cmake_integration(cmake_fetch_content SOURCE_DIR cmake_fetch_content)
else()
    message("Ignored ${NAME} (CMake version is less than 3.11)")
endif()

file(REMOVE_RECURSE "${TEMP_DIR}")
