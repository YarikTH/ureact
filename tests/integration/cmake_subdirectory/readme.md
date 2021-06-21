# CMake integration using add_subdirectory

If you have the entire **µReact** repository available (as a git submodule or just as files)
you could include it in your CMake build.

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

In this example, `add_subdirectory` points to the project root, because
linking it can potentially cause problems. The second argument of `add_subdirectory`
needed because the subdirectory points outside this example's root.

---------------

[Back to Integration](../)

[Home](../../../doc/readme.md#reference)
