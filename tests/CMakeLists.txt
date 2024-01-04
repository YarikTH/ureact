#
#         Copyright (C) 2020-2023 Yaroslav Krylov.
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
    URL "https://github.com/catchorg/Catch2/archive/refs/tags/v3.4.0.tar.gz"
    URL_HASH MD5=2c802a4938ed842e2942c60d1d231bb7
)

FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
    nanobench
    URL "https://github.com/martinus/nanobench/archive/refs/tags/v4.3.11.tar.gz"
    URL_HASH MD5=0a91801ca6bf2146d92887ef0018425c
)

FetchContent_MakeAvailable(nanobench)
# gersemi: on

add_subdirectory(self_contained)
add_subdirectory(src)
