#!/bin/bash

#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)

COLOR_RED="\e[1;31m"
COLOR_GREEN="\e[1;32m"
COLOR_DEFAULT="\e[0m"

function F_report_error {
    echo -e "${COLOR_RED}Check failed! $1${COLOR_DEFAULT}"
    exit 2
}

function F_check_error {
    if [ "$?" != "0" ]; then
        F_report_error "$1"
    fi
}

function get_abspath { echo "$(readlink -f "$1")"; }

function F_detect_tools() {
    # Based on https://gist.github.com/leilee/1d0915a583f8f29414cc21cd86e7151b
    # Variable that will hold the name of the clang-format command
    CLANG_FORMAT_TOOL=""

    # Some distros just call it clang-format. Others (e.g. Ubuntu) are insistent
    # that the version number be part of the command. We prefer clang-format if
    # that's present, otherwise we work backwards from highest version to lowest
    # version.
    for clangfmt in clang-format{-{13,12,11},-{13,12,11}.{9,8,7,6,5,4,3,2,1,0}}; do
        if which "$clangfmt" &>/dev/null; then
            CLANG_FORMAT_TOOL="$clangfmt"
            break
        fi
    done

    if [ "${CLANG_FORMAT_TOOL}" == "" ]; then
        echo "clang-format tool is not found. Version 9 or greater is required."
        echo "You can install clang-format from apt:"
        echo "\$ sudo apt install clang-format-9"
        echo ""
        TOOLS_NOT_FOUND=1
    fi

    # Variable that will hold the name of the gersemi command
    GERSEMI_TOOL=""

    if which "gersemi" &>/dev/null; then
        GERSEMI_TOOL="gersemi"
    fi

    if [ "${GERSEMI_TOOL}" == "" ]; then
        echo "gersemi tool is not found."
        echo "You can install gersemi from PyPI:"
        echo "\$ pip3 install gersemi"
        echo ""
        TOOLS_NOT_FOUND=1
    fi

    if [ "${TOOLS_NOT_FOUND}" == "1" ]; then
        exit 2
    fi
}

function F_show_version() {
    ${CLANG_FORMAT_TOOL} --version
    ${GERSEMI_TOOL} --version | head -n 1
}

function F_show_usage() {
    echo "Usage: `basename $0` [--check] [--help] [--version] [src [src ...]]"
    echo ""
    echo "A formatter to make your cpp and CMake code the real treasure."
    echo ""
    echo "positional arguments:"
    echo "  src                   File or directory to format. If only - is provided input is taken"
    echo "                        from stdin instead"
    echo ""
    echo "modes:"
    echo "  -c, --check           Check if files require reformatting. Return 0 when there's nothing"
    echo "                        to reformat, return 1 when some files would be reformatted"
    echo "  -v, --version         Show version."
    echo "  -h, --help            Show this help message and exit."
}

function F_report_usage_error {
    echo -e "${COLOR_RED}$1${COLOR_DEFAULT}"
    F_show_usage
    exit 2
}

function F_check_if_path_match_patterns {
    local file
    file="$1"
    shift
    for mask in $@; do
        if [[ "$file" == *${mask} ]]; then
            return 0
        fi
    done
    return 1
}

CMAKE_PATTERNS=(.cmake CMakeLists.txt)
CPP_PATTERNS=(.c .cpp .h .hpp)

function F_detect_file_type {
    F_check_if_path_match_patterns "$FILE" ${CMAKE_PATTERNS[@]}
    if [[ "$?" == "0" ]]; then
        echo "cmake"
        return
    fi

    F_check_if_path_match_patterns "$FILE" ${CPP_PATTERNS[@]}
    if [[ "$?" == "0" ]]; then
        echo "cpp"
        return
    fi

    echo "unknown"
}

F_detect_tools

CHECK_IF_FORMATTED=0

POSITIONAL=()
while [[ $# -gt 0 ]]; do
    key="$1"

    case $key in
        -h|--help)
            F_show_usage
            exit 0
            ;;
        -v|--version)
            F_show_version
            exit 0
            ;;
        -c|--check)
            CHECK_IF_FORMATTED=1
            shift # past argument
            ;;
        *)    # unknown option
            POSITIONAL+=("$1") # save it in an array for later
            shift # past argument
            ;;
    esac
done
set -- "${POSITIONAL[@]}" # restore positional parameters

CMAKE_FILES_TO_CHECK=()
CPP_FILES_TO_CHECK=()
for FILE in $@; do
    type=`F_detect_file_type ${FILE}`
    if [[ "${type}" == "cmake" ]]; then
        CMAKE_FILES_TO_CHECK+=("${FILE}")
    elif [[ "${type}" == "cpp" ]]; then
        CPP_FILES_TO_CHECK+=("${FILE}")
    fi
done

if [[ "${CHECK_IF_FORMATTED}" == "1" ]]; then
    if [[ "${#CPP_FILES_TO_CHECK[@]}" != "0" ]]; then
        ${CLANG_FORMAT_TOOL} --Werror --dry-run --verbose ${CPP_FILES_TO_CHECK[@]}
        if [[ "$?" == "1" ]]; then
            CHECK_IS_FAILED=1
        fi
    fi

    if [[ "${#CMAKE_FILES_TO_CHECK[@]}" != "0" ]]; then
        ${GERSEMI_TOOL} --check ${CMAKE_FILES_TO_CHECK[@]}
        if [[ "$?" == "1" ]]; then
            CHECK_IS_FAILED=1
        fi
    fi

    if [[ "${CHECK_IS_FAILED}" == "1" ]]; then
        echo -e "${COLOR_RED}Check is failed! See the errors above.${COLOR_DEFAULT}"
        exit 1
    else
        echo -e "${COLOR_GREEN}Check is succeeded!${COLOR_DEFAULT}"
        exit 0
    fi
else
    if [[ "${#CPP_FILES_TO_CHECK[@]}" != "0" ]]; then
        ${CLANG_FORMAT_TOOL} --Werror -i ${CPP_FILES_TO_CHECK[@]}
    fi
    if [[ "${#CMAKE_FILES_TO_CHECK[@]}" != "0" ]]; then
        ${GERSEMI_TOOL} --in-place -q ${CMAKE_FILES_TO_CHECK[@]}
    fi
    exit 0
fi
