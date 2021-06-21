# CMake integration using FetchContent

Since CMake v3.11,
[FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) can
be used to automatically download the repository as a dependency at configure time.

Despite [ExternalProject](../cmake_external_project) as a result, we get
`ureact::ureact` target that should be linked to the needed library.

## Running test

To run the test you should use a common CMake pattern:

```console
mkdir build
cd build
cmake ..
cmake --build .
./cmake_test && echo SUCCESS || echo FAIL
```

---------------

[Back to Integration](../)

[Home](../../../doc/readme.md#reference)
