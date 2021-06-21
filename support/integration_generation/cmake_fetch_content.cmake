# gersemi: off
include(FetchContent)

FetchContent_Declare(
    ureact
    GIT_REPOSITORY https://github.com/YarikTH/ureact.git
    GIT_TAG main
)

FetchContent_GetProperties(ureact)
if(NOT ureact_POPULATED)
    FetchContent_Populate(ureact)
    add_subdirectory(${ureact_SOURCE_DIR} ${ureact_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
# gersemi: on
