#
#         Copyright (C) 2020-2024 Yaroslav Krylov.
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
	@echo "* docs - generate documentation in doc/html"
	@echo "* docs_deploy - deploy documentation to gh-pages"
	@echo "* amalgamated - generate ureact_amalgamated.hpp (does prettify first)"
	@echo "* integration_check - check library integrations"
	@echo "* integration_update - generate integration files"


##########################################################################
# Prettify
##########################################################################
PRETTY_PY_COMMAND = /usr/bin/env python3 ./support/thirdparty/pretty.py/pretty.py
PRETTY_PY_OPTIONS = --clang-format clang-format-14
PRETTY_PY_OPTIONS += --exclude='tests/thirdparty'
PRETTY_PY_OPTIONS += --exclude='tests/integration'
PRETTY_PY_OPTIONS += --exclude='support/thirdparty'
PRETTY_PY_OPTIONS += --exclude='single_include'
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
# Documentation
##########################################################################
ASCIIDOCTOR_COMMON_ARGS = --source-dir ./doc
ASCIIDOCTOR_COMMON_ARGS += --destination-dir ./doc/html

ASCIIDOCTOR_HTML_ARGS = --out-file "index.html"
ASCIIDOCTOR_HTML_ARGS += --backend "html5"
ASCIIDOCTOR_HTML_ARGS += --doctype "article"
ASCIIDOCTOR_HTML_ARGS += --attribute "stylesheet=zajo-dark.css"
ASCIIDOCTOR_HTML_ARGS += --attribute "linkcss"

ASCIIDOCTOR_PDF_ARGS = --out-file "ureact.pdf"
ASCIIDOCTOR_PDF_ARGS += --backend "pdf"
ASCIIDOCTOR_PDF_ARGS += --doctype "book"
ASCIIDOCTOR_PDF_ARGS += --require "asciidoctor-pdf"
ASCIIDOCTOR_PDF_ARGS += --attribute "pdf-themesdir=./doc"
ASCIIDOCTOR_PDF_ARGS += --attribute "pdf-theme=ureact"

# generate documentation in doc/html
docs:
	@asciidoctor $(ASCIIDOCTOR_COMMON_ARGS) $(ASCIIDOCTOR_HTML_ARGS) ./doc/readme.adoc
	@asciidoctor $(ASCIIDOCTOR_COMMON_ARGS) $(ASCIIDOCTOR_PDF_ARGS) ./doc/readme.adoc
	@cp ./doc/skin.png ./doc/zajo-light.css ./doc/rouge-github.css ./doc/html

# deploy documentation to gh-pages
# See https://gist.github.com/cobyism/4730490
docs_deploy:
	@echo "Deploying documentation to GitHub pages. Make sure latest documentation is already committed"
	@git push origin :gh-pages && git subtree push --prefix doc/html/ origin gh-pages


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

