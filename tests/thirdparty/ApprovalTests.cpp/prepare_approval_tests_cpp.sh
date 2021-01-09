#! /bin/bash -e

APPROVAL_TESTS_VERSION="v.10.6.0"
APPROVAL_TESTS_ROOT=include
APPROVAL_TESTS_FILE_NAME=ApprovalTests.${APPROVAL_TESTS_VERSION}.hpp

APPROVAL_TESTS_URL=https://github.com/approvals/ApprovalTests.cpp/releases/download/${APPROVAL_TESTS_VERSION}/${APPROVAL_TESTS_FILE_NAME}

function download_approval_tests()
(
    rm -rf ${APPROVAL_TESTS_ROOT}
    (mkdir -p ${APPROVAL_TESTS_ROOT} && cd ${APPROVAL_TESTS_ROOT} && wget ${APPROVAL_TESTS_URL} && mv ${APPROVAL_TESTS_FILE_NAME} ApprovalTests.hpp )
)

function main()
{
    download_approval_tests
}

main
echo "Done!"













