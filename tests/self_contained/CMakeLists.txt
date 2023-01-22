#
#         Copyright (C) 2020-2023 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
add_executable(ureact_self_contained_headers_test main.cpp)

function(ureact_setup_self_contained_test_sources)
    file(
        GLOB_RECURSE ureact_headers
        RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
        "../../include/ureact/*.hpp"
    )
    foreach(header IN LISTS ureact_headers)
        string(MAKE_C_IDENTIFIER "${header}" _out)
        set(src_filename
            "${CMAKE_CURRENT_BINARY_DIR}/check_if_${_out}_self_contained.cpp"
        )
        file(WRITE "${src_filename}" "#include <${header}>\n")
        target_sources(
            ureact_self_contained_headers_test
            PRIVATE "${src_filename}"
        )
    endforeach()
endfunction()

ureact_setup_self_contained_test_sources()

target_link_libraries(ureact_self_contained_headers_test PRIVATE ureact::ureact)

target_compile_options(
    ureact_self_contained_headers_test
    PRIVATE ${UREACT_WARNING_OPTION}
)

add_test(
    NAME ureact_self_contained_headers_test
    COMMAND ureact_self_contained_headers_test
)
