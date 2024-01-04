#
#         Copyright (C) 2020-2024 Yaroslav Krylov.
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
    URL "https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.1.tar.gz"
    URL_HASH MD5=3b666cc058b84e9af3431cad9649c1af
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
