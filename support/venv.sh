#!/bin/bash

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

SELF_DIR="$( dirname "$(readlink -f "$0")" )"

python3 -m venv "${SELF_DIR}/venv"
    F_check_error "Can't create venv. Try to install it with\n* apt-get install python-venv"

source "${SELF_DIR}/venv/bin/activate"
    F_check_error "Can't activate venv"

pip3 install -qr "${SELF_DIR}/requirements.txt"
    F_check_error "Can't install python requirements"

exec $@
