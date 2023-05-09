#
#         Copyright (C) 2020-2023 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required(VERSION 3.14)

# gersemi: off
include(FetchContent)

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY "https://github.com/catchorg/Catch2.git"
    GIT_TAG "v3.3.2"
)

FetchContent_MakeAvailable(Catch2)
# gersemi: on

add_subdirectory(self_contained)
add_subdirectory(src)
