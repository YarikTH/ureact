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
	@echo "* integration_check - check library integrations"
	@echo "* integration_update - generate integration files"
	@echo "* changelog - generate ChangeLog file"


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
	@$(PRETTY_PY_COMMAND) $(PRETTY_PY_OPTIONS) --check -

# prettify all cmake and c++ files
pretty:
	@$(PRETTY_PY_COMMAND) $(PRETTY_PY_OPTIONS) -


##########################################################################
# Integration
##########################################################################

# check library integrations
integration_check:
	@cmake -P ./tests/integration/test_integration.cmake

# generate integration files
integration_update:
	@cmake -P ./support/integration_generation/generate.cmake


##########################################################################
# ChangeLog
##########################################################################

# Create a ChangeLog based on the git log using the GitHub Changelog Generator
# (<https://github.com/github-changelog-generator/github-changelog-generator>).

# variable to control the diffs between the last released version and the current repository state
NEXT_VERSION ?= "unreleased"

changelog:
	github_changelog_generator -o CHANGELOG.md --user YarikTH --project ureact --simple-list --release-url https://github.com/YarikTH/ureact/releases/tag/%s --release-branch dev --future-release $(NEXT_VERSION)
	@$(SED) -i 's|https://github.com/YarikTH/ureact/releases/tag/HEAD|https://github.com/YarikTH/ureact/tree/HEAD|' CHANGELOG.md
	@$(SED) -i '2i All notable changes to this project will be documented in this file. This project adheres to [Semantic Versioning](http://semver.org/).' CHANGELOG.md

