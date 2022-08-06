#
#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
add_library(UreactDoctest INTERFACE)

add_library(ureact::doctest ALIAS UreactDoctest)

target_include_directories(UreactDoctest SYSTEM INTERFACE include)

target_compile_definitions(UreactDoctest INTERFACE DOCTEST_CONFIG_REQUIRE_STRINGIFICATION_FOR_ALL_USED_TYPES)
