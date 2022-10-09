#
#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
#
##########################################################################
# configuration
##########################################################################

# find GNU sed to use `-i` parameter
SED:=$(shell command -v gsed || which sed)

##########################################################################
# documentation of the Makefile's targets
##########################################################################

# main target
all:
	@echo "This Makefile is for the maintenance of the repository, not for building."
	@echo "Supported targets:"
	@echo "* pretty_check - check if all cmake and c++ files are properly formatted"
	@echo "* pretty - prettify all cmake and c++ files"
	@echo "* amalgamated - generate ureact_amalgamated.hpp (does prettify first)"
	@echo "* integration_check - check library integrations"
	@echo "* integration_update - generate integration files"


##########################################################################
# Prettify
##########################################################################
PRETTY_PY_COMMAND = /usr/bin/env python3 ./support/thirdparty/pretty.py/pretty.py
PRETTY_PY_OPTIONS = --clang-format clang-format-11
PRETTY_PY_OPTIONS += --exclude='tests/thirdparty'
PRETTY_PY_OPTIONS += --exclude='tests/integration'
PRETTY_PY_OPTIONS += --exclude='support/thirdparty'
#TODO: support adding of additional path after excluding
#PRETTY_PY_OPTIONS += "--include='tests/integration/test_integration.cmake'"
#PRETTY_PY_OPTIONS += "--include='tests/thirdparty/doctest/CMakeLists.txt'"

# check if all cmake and c++ files are properly formatted
pretty_check:
	@./support/venv.sh $(PRETTY_PY_COMMAND) $(PRETTY_PY_OPTIONS) --check -

# prettify all cmake and c++ files
pretty:
	@./support/venv.sh $(PRETTY_PY_COMMAND) $(PRETTY_PY_OPTIONS) -


##########################################################################
# Amalgamated
##########################################################################

# generate ureact_amalgamated.hpp (does prettify first)
amalgamated: pretty
	@./support/venv.sh /usr/bin/env python3 ./support/generate_amalgamated_file.py


##########################################################################
# Integration
##########################################################################

# check library integrations
integration_check:
	@cmake -P ./tests/integration/test_integration.cmake

# generate integration files
integration_update:
	@cmake -P ./support/integration_generation/generate.cmake

