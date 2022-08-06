#
#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
add_executable(
    ureact_test #
    main.cpp
    legacy.cpp
)

target_link_libraries(ureact_test PRIVATE ureact::ureact ureact::doctest)

target_compile_options(ureact_test PRIVATE ${UREACT_WARNING_OPTION})

add_test(NAME ureact_test COMMAND ureact_test)
