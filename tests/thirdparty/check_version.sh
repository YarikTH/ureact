#!/bin/bash

COLOR_RED="\e[1;31m"
COLOR_GREEN="\e[1;32m"
COLOR_DEFAULT="\e[0m"
OK_STATUS="[${COLOR_GREEN}✓${COLOR_DEFAULT}]"
FAIL_STATUS="[${COLOR_RED}✖${COLOR_DEFAULT}]"

function F_report_error {
    echo -e >&2 "${COLOR_RED}Check failed! $1${COLOR_DEFAULT}"
    exit 1
}

function F_check_error {
    if [ "$?" != "0" ]; then
        F_report_error "$1"
    fi
}

SELF_DIR="$( dirname "$(readlink -f "$0")" )"

printf "    %-30s %-10s %-10s\n" "Library" "Current" "Latest"
for META_JSON_PATH in `find . -maxdepth 2 -mindepth 2 -type f -name 'meta.json' | sort`; do
    HOSTING=`jq -r .hosting ${META_JSON_PATH}`
    LIB=`jq -r .path ${META_JSON_PATH}`
    CURRENT_VERSION=`jq -r .current_version ${META_JSON_PATH}`

    LATEST_VERSION=`curl --silent "https://api.github.com/repos/${LIB}/releases/latest" | jq -r .tag_name`

    if [ "${CURRENT_VERSION}" != "${LATEST_VERSION}" ]; then
        STATUS="${FAIL_STATUS}"
        CHECK_FAILED="1"
    else
        STATUS="${OK_STATUS}"
    fi

    printf "${STATUS} %-30s %-10s %-10s\n" "${LIB}" "${CURRENT_VERSION}" "${LATEST_VERSION}"
done
