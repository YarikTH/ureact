# CMake integration using find_package

If you have installed version of **µReact** (manually or via packet manager)
then a proper way for CMake is using `find_package`.
As a result, we get the `ureact::ureact` target that should be linked to
the needed library.

## Install ureact locally

To run `find_package` test we need installed version on **µReact**. The easies way
to get it - install ureact locally, using CMake:

```console
cmake -B build_ureact -DUREACT_TEST=n -DCMAKE_INSTALL_PREFIX=install_ureact -S <path_to_ureact_root>
cmake --build build_ureact --target install
```

## Running test

To run the test you should use a common CMake pattern:

```console
mkdir build
cd build
cmake ..
cmake --build .
./cmake_test && echo SUCCESS || echo FAIL
```

If you installed ureact locally, then add `CMAKE_PREFIX_PATH` at the CMake
configuration time:

```console
cmake -DCMAKE_PREFIX_PATH=install_ureact ..
```

---------------

[Back to Integration](../)

[Home](../../../doc/readme.md#reference)
