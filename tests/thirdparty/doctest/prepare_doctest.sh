#! /bin/bash -e

DOCTEST_VERSION="2.4.4"
DOCTEST_ROOT=include
DOCTEST_FILE_NAME=doctest.h
DOCTEST_URL=https://raw.githubusercontent.com/onqtam/doctest/${DOCTEST_VERSION}/doctest/${DOCTEST_FILE_NAME}

function download_doctest()
(
    rm -rf ${DOCTEST_ROOT}
    (mkdir -p ${DOCTEST_ROOT} && cd ${DOCTEST_ROOT} && wget ${DOCTEST_URL})
)

function main()
{
    download_doctest
}

main
echo "Done!"
