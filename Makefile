#
#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
#
##########################################################################
# source files
##########################################################################

# the list of CMakeLists.txt
CMAKE_FILES_TO_FORMAT=$(shell find . -type f -name CMakeLists.txt -o -name '*.cmake' | grep -v './cmake-build' | grep -v 'integration_main.cmake' | sort)

INCLUDE_FILES_TO_FORMAT=$(shell find ./include -type f -name '*.hpp' | sort)
TEST_FILES_TO_FORMAT=$(shell find ./tests -type f -name '*.h' -or -name '*.hpp' -or -name '*.c' -or -name '*.cpp' | grep -v 'thirdparty' | sort)
CPP_FILES_TO_FORMAT=${INCLUDE_FILES_TO_FORMAT} ${TEST_FILES_TO_FORMAT}

FILES_TO_FORMAT=${CMAKE_FILES_TO_FORMAT} ${CPP_FILES_TO_FORMAT}

##########################################################################
# documentation of the Makefile's targets
##########################################################################

# main target
all:
	@echo "This Makefile is for the maintenance of the repository, not for building."
	@echo "Supported targets:"
	@echo "* pretty_version - output version of formatting tools"
	@echo "* pretty_check - check if all cmake and c++ files are properly formatted"
	@echo "* pretty - prettify all cmake and c++ files"
	@echo "* integration_check - check library integrations"
	@echo "* integration_update - generate integration files"

##########################################################################
# Prettify
##########################################################################

# output version of formatting tools
pretty_version:
	@./support/pretty.sh --version

# check if all cmake and c++ files are properly formatted
pretty_check:
	@./support/pretty.sh --check $(FILES_TO_FORMAT)

# prettify all cmake and c++ files
pretty:
	@./support/pretty.sh $(FILES_TO_FORMAT)

##########################################################################
# Integration
##########################################################################

# check library integrations
integration_check:
	@cmake -P ./tests/integration/test_integration.cmake

# generate integration files
integration_update:
	@cmake -P ./support/integration_generation/generate.cmake
