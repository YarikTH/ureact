# Integration

<!--
TODO use https://github.com/nlohmann/json#integration
as a reference
-->

The latest released version of **µReact** can be obtained from here: https://raw.githubusercontent.com/YarikTH/ureact/main/include/ureact/ureact.hpp

You can substitute ```main``` with ```dev``` or a tag like ```0.1.0``` for a specific version in the URL above.

You need to add

```cpp
#include <ureact/ureact.hpp>
```

to the files you want to process use µReact and set the necessary switches to enable C++11 (e.g., `-std=c++11` for GCC and Clang).

## CMake

You can use the `ureact::ureact` interface target in CMake.  This target populates the appropriate usage requirements for `INTERFACE_INCLUDE_DIRECTORIES` to point to the appropriate include directories and `INTERFACE_COMPILE_FEATURES` for the necessary C++11 flags.

### External

**µReact** is easiest to use as a single file inside your own repository. Then the following minimal example will work:

```cmake
cmake_minimum_required(VERSION 3.8)
project(cmake_test VERSION 0.0.1 LANGUAGES CXX)

# Prepare ureact for other targets to use
find_package(ureact REQUIRED)

# Make executable
add_executable(my_exe main.cpp)
target_link_libraries(my_exe PRIVATE ureact::ureact)
```

### ExternalProject

You can use the following CMake snippet to automatically fetch the entire **µReact** repository from github and configure it as an external project:

```cmake
include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    ureact
    PREFIX ${CMAKE_BINARY_DIR}/ureact
    GIT_REPOSITORY https://github.com/YarikTH/ureact.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

# Expose required variable (UREACT_INCLUDE_DIR) to parent scope
ExternalProject_Get_Property(ureact source_dir)
set(UREACT_INCLUDE_DIR ${source_dir}/ureact CACHE INTERNAL "Path to include folder for ureact")
```

And later you'll be able to use the **µReact** include directory like this:

```cmake
target_include_directories(my_target PUBLIC ${UREACT_INCLUDE_DIR})
```

### add_subdirectory

If you have the entire **µReact** repository available (as a submodule or just as files) you could also include it in your CMake build:

```cmake
add_subdirectory(path/to/ureact)

add_executable(my_exe src_1.cpp src_2.cpp ...)
target_link_libraries(my_exe ureact::ureact)
```

### FetchContent

Since CMake v3.11,
[FetchContent](https://cmake.org/cmake/help/v3.11/module/FetchContent.html) can
be used to automatically download the repository as a dependency at configure time.

Example:
```cmake
include(FetchContent)

FetchContent_Declare(ureact
  GIT_REPOSITORY https://github.com/YarikTH/ureact.git
  GIT_TAG 0.1.0)

FetchContent_GetProperties(ureact)
if(NOT ureact_POPULATED)
  FetchContent_Populate(ureact)
  add_subdirectory(${ureact_SOURCE_DIR} ${ureact_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_link_libraries(foo PRIVATE ureact::ureact)
```

### Install

The ```CMakeLists.txt``` file of the **µReact** repository has ```install()``` commands, so you could also use **µReact** as a package.

## Package Managers

😅 Unfortunately there are no package managers supported at this moment.

---------------

[Home](readme.md#reference)
