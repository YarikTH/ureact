#
#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
add_library(UreactDoctest INTERFACE)
target_include_directories(UreactDoctest SYSTEM INTERFACE include)
add_library(ureact::doctest ALIAS UreactDoctest)
