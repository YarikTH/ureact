# CMake integration using copied header

This example contains a "copy" of the `ureact.hpp` header.

To use it properly we define a CMake interface target `ureact::ureact`.
The target points to its include directory and tells that library or executable
that want to use it should be compiled with at least c++17 standard.

Then we can link our target to any target that wants to use **µReact** and that's all.

You can always go an easy way and drop the `ureact.hpp` header into your include
directory and call it a day.

## Running test

To run the test you should use a common CMake pattern:

```console
mkdir build
cd build
cmake ..
cmake --build .
./cmake_test && echo SUCCESS || echo FAIL
```

## Notes

* In this example `ureact.hpp` is linked instead of copied, because it's
a bad idea to have several copies of a file in the repository.

---------------

[Back to Integration](../)

[Home](../../../doc/readme.md#reference)
