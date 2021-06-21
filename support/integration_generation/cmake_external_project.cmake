# gersemi: off
include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    get_ureact
    PREFIX ${CMAKE_BINARY_DIR}/ureact
    GIT_REPOSITORY https://github.com/YarikTH/ureact.git
    GIT_TAG main
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

# Expose required variable UREACT_INCLUDE_DIR
ExternalProject_Get_Property(get_ureact source_dir)
set(UREACT_INCLUDE_DIR ${source_dir}/include CACHE INTERNAL "Path to include folder for ureact")

# Create result folder at configuration time to make CMake happy
file(MAKE_DIRECTORY ${UREACT_INCLUDE_DIR})

# Create ureact::ureact interface target
add_library(ureact_ureact INTERFACE)
add_library(ureact::ureact ALIAS ureact_ureact)
target_include_directories(ureact_ureact INTERFACE ${UREACT_INCLUDE_DIR})
target_compile_features(ureact_ureact INTERFACE cxx_std_11)

# Add dependency with get_ureact so, ureact will be downloaded before using
add_dependencies(ureact_ureact get_ureact)

# gersemi: on
