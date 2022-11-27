# µReact

<p align="center"><img src="support/data/logo/logo_1.svg"></p>

[![Ubuntu](https://github.com/YarikTH/ureact/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/YarikTH/ureact/actions/workflows/ubuntu.yml)
[![Windows](https://github.com/YarikTH/ureact/actions/workflows/windows.yml/badge.svg)](https://github.com/YarikTH/ureact/actions/workflows/windows.yml)
[![MacOS](https://github.com/YarikTH/ureact/actions/workflows/macos.yml/badge.svg)](https://github.com/YarikTH/ureact/actions/workflows/macos.yml)
[![CodeQL](https://github.com/YarikTH/ureact/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/YarikTH/ureact/actions/workflows/codeql-analysis.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/7ad08869b5a948bb917deda4ae004064)](https://www.codacy.com/gh/YarikTH/ureact/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=YarikTH/ureact&amp;utm_campaign=Badge_Grade)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/YarikTH/ureact.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/YarikTH/ureact/context:cpp)

[![Standard](https://img.shields.io/badge/c%2B%2B-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![Type](https://img.shields.io/badge/type-header--only-blue.svg)](.)
[![GitHub license](https://img.shields.io/github/license/YarikTH/ureact?color=blue)](https://raw.githubusercontent.com/YarikTH/ureact/main/LICENSE_1_0.txt)
[![download](https://img.shields.io/badge/download%20%20-link-blue.svg)](https://raw.githubusercontent.com/YarikTH/ureact/master/include/ureact/ureact.hpp)
[![Docs](https://img.shields.io/badge/docs-markdown-blue.svg)](https://github.com/YarikTH/ureact/tree/main/doc/readme.md)
[![GitHub Releases](https://img.shields.io/github/release/YarikTH/ureact.svg)](https://github.com/YarikTH/ureact/releases)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](.)

**µReact** is an open-source header-only [functional reactive programming](https://en.wikipedia.org/wiki/Functional_reactive_programming) library for C++17.

❗️ **This library is a work-in-progress. It should not be considered release quality yet and its API might still change.** \
**However, it works perfectly fine and can be already used with small future changes in mind.** \
**Feedback is strongly required and appreciated to stabilize the API and achieve the first major release.**

[Documentation](doc/readme.md)

## Features

* Update minimality: nothing is re-calculated or processed unnecessarily
* Glitch freedom: no transiently inconsistent data sets
* Externally synchronized (not thread-safe) by design: it allows not to pay
  for what you don't use and to have very determined consistent behaviour
* Ease of use: small self-contained single header code base, no external dependencies,
  permissive Boost Software License [license](LICENSE_1_0.txt)
* Really easy to get started: it's just 1 header file - see the [tutorial](doc/tutorial.md)
* Reliability: the library has an extensive set of [tests](tests/src)
* Portability: continuously tested under Windows, MacOS and Ubuntu using **MSVC**/**GCC**/**Clang**
* Clean warning-free codebase even on the [**most aggressive**](support/cmake/set_warning_flags.cmake) warning levels for **MSVC**/**GCC**/**Clang**
* Doesn't pollute the global namespace (everything is in namespace ```ureact```) and doesn't drag **any** headers with it

## Examples

**Basic usage** ([run](https://godbolt.org/z/45rqEnPr7))

```cpp
ureact::context ctx;

// Declaring reactive variables. We can reassign their values later
ureact::var_signal<int> b = make_var( ctx, 1 );
ureact::var_signal<int> c = make_var( ctx, 2 );

// Defining reactive signal using overloaded operator
// Its value will be updated each time its dependencies are changed
ureact::signal<int> a = b + c;

std::cout << "a (init): " << a.get() << "\n"; // 3

// Assign a new value to 'b'. Value of 'a' is recalculated automatically
b <<= 10;
std::cout << "a  (new): " << a.get() << "\n"; // 12
```

## License

<img align="right" src="https://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

This software is licensed under the [Boost Software License 1.0](https://opensource.org/licenses/BSL-1.0):

Copyright &copy; 2014-2017 [Sebastian Jeckel](https://github.com/snakster)

Copyright &copy; 2020-2022 [Krylov Yaroslav](https://github.com/YarikTH)

Permission is hereby granted, free of charge, to any person or organization obtaining a copy of the software and accompanying documentation covered by this license (the "Software") to use, reproduce, display, distribute, execute, and transmit the Software, and to prepare derivative works of the Software, and to permit third-parties to whom the Software is furnished to do so, all subject to the following:

The copyright notices in the Software and this entire statement, including the above license grant, this restriction and the following disclaimer, must be included in all copies of the Software, in whole or in part, and all derivative works of the Software, unless such copies or derivative works are solely in the form of machine-executable object code generated by a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

* * *

This software started out as a fork of the [cpp.react](https://github.com/schlangster/cpp.react) by [Sebastian Jeckel](https://github.com/snakster) (schlangster@gmail.com), [BSL 1.0 licensed](http://www.boost.org/LICENSE_1_0.txt).

## Used third-party tools

The library itself consists of a single header file licensed under the Boost Software License license. However, it is built, tested, documented, and whatnot using a lot of third-party tools and services. Thanks a lot!

* [**CMake**](https://cmake.org) for build automation
* [**ClangFormat**](https://clang.llvm.org/docs/ClangFormat.html) for automatic source code indentation
* [**gersemi**](https://github.com/blankspruce/gersemi) for automatic CMake code indentation
* [**doctest**](https://github.com/onqtam/doctest) for the unit tests
* [**nanobench**](https://github.com/martinus/nanobench) for the benchmarks

## Contact

If you have questions regarding the library, I would like to invite you to [open an issue at GitHub](https://github.com/YarikTH/ureact/issues/new/choose). Please describe your request, problem, or question as detailed as possible, and also mention the version of the library you are using as well as the version of your compiler and operating system. Opening an issue at GitHub allows other users and contributors to this library to collaborate.
