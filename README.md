# µReact

<p align="center"><img src="support/data/logo/logo_1.svg"></p>

[![Ubuntu](https://github.com/YarikTH/ureact/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/YarikTH/ureact/actions/workflows/ubuntu.yml)
[![Windows](https://github.com/YarikTH/ureact/actions/workflows/windows.yml/badge.svg)](https://github.com/YarikTH/ureact/actions/workflows/windows.yml)
[![MacOS](https://github.com/YarikTH/ureact/actions/workflows/macos.yml/badge.svg)](https://github.com/YarikTH/ureact/actions/workflows/macos.yml)
[![CodeQL](https://github.com/YarikTH/ureact/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/YarikTH/ureact/actions/workflows/codeql-analysis.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/7ad08869b5a948bb917deda4ae004064)](https://www.codacy.com/gh/YarikTH/ureact/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=YarikTH/ureact&amp;utm_campaign=Badge_Grade)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/YarikTH/ureact.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/YarikTH/ureact/context:cpp)

[![Standard](https://img.shields.io/badge/c%2B%2B-11/14/17/20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standardization)
[![Type](https://img.shields.io/badge/type-single--header-blue.svg)](.)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/YarikTH/ureact/main/LICENSE.MIT)
[![download](https://img.shields.io/badge/download%20%20-link-blue.svg)](https://raw.githubusercontent.com/YarikTH/ureact/master/include/ureact/ureact.hpp)
[![Docs](https://img.shields.io/badge/docs-markdown-blue.svg)](https://github.com/YarikTH/ureact/tree/main/doc/readme.md)
[![GitHub Releases](https://img.shields.io/github/release/YarikTH/ureact.svg)](https://github.com/YarikTH/ureact/releases)
[![GitHub Issues](https://img.shields.io/github/issues/YarikTH/ureact.svg)](https://github.com/YarikTH/ureact/issues)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](.)

**µReact** is an open-source minimalistic single-header [reactive programming](https://en.wikipedia.org/wiki/Reactive_programming) library for C++11.

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
  permissive MIT [license](LICENSE.MIT)
* Really easy to get started: it's just 1 header file - see the [tutorial](doc/tutorial.md)
* Reliability: the library has an extensive set of [tests](tests/src)
* Portability: continuously tested under Windows, MacOS and Ubuntu using **MSVC**/**GCC**/**Clang**
* Clean warning-free codebase even on the [**most aggressive**](support/cmake/set_warning_flags.cmake) warning levels for **MSVC**/**GCC**/**Clang**
* Doesn't pollute the global namespace (everything is in namespace ```ureact```) and doesn't drag **any** headers with it

## Examples

**Basic usage** ([run](https://godbolt.org/z/WEbKWojz5))

```cpp
ureact::context ctx;

// Declaring reactive variables. We can reassign their values later
ureact::var_signal<int> b = ctx.make_var(1);
ureact::var_signal<int> c = ctx.make_var(2);

// Declaring reactive signal using overloaded operator
// Its value will be updated each time its dependencies are changed
ureact::signal<int> a = b + c;

std::cout << "a (init): " << a.value() << "\n"; // 3

// Assign a new value to 'b'. Value of 'a' is recalculated automatically
b <<= 10;
std::cout << "a  (new): " << a.value() << "\n"; // 12
```

**Complex signals** ([run](https://godbolt.org/z/ns1rPM7eK))

```cpp
ureact::context ctx;

ureact::var_signal<int> base = ctx.make_var(1);
ureact::var_signal<int> exp = ctx.make_var(3);

// Declaring reactive signal with formula
// Its value will be recalculated according to the given function
ureact::signal<double> result = make_signal( with(base, exp) , std::pow<int, int> );

// Alternative form of make_signal using operator ->*
ureact::signal<std::string> expression = with(base, exp, result) ->*
    []( int base, int exp, int result ){
        return std::to_string(base) + "^" + std::to_string(exp)
            + " == " + std::to_string(result);
    };

std::cout << expression.value() << "\n"; // 1^3 == 1

base <<= 2;
std::cout << expression.value() << "\n"; // 2^3 == 8

exp <<= 0;
std::cout << expression.value() << "\n"; // 2^0 == 1
```

**Observers** ([run](https://godbolt.org/z/fYYMaTdPj))

```cpp
ureact::context ctx;

ureact::var_signal<int> a = ctx.make_var(1);
ureact::signal<int> abs_a = with(a) ->* [](int a){ return std::abs(a); };
ureact::signal<int> abs_a_x2 = abs_a * 2;

// Declaring reactive observers
// They execute given functor only when the observed value is changed
observe(a,        []( int a ){        std::cout << "  a -> "        << a << "\n"; });
observe(abs_a,    []( int abs_a ){    std::cout << "  abs(a) -> "    << abs_a << "\n"; });
observe(abs_a_x2, []( int abs_a_x2 ){ std::cout << "  abs(a)*2 -> " << abs_a_x2 << "\n"; });

// All values change their values so all of them is notified
std::cout << "a <<= 3\n";
a <<= 3;

// Only change of 'a' is notified
// Value of 'abs_a' is not changed after recalculation, so it is not notified
// Value 'abs_a_x2' is not even recalculated because the value of 'abs_a' is not changed
std::cout << "a <<= -3\n";
a <<= -3;
```

Output:
```
a <<= 3
  a -> 3
  abs(a) -> 3
  abs(a)*2 -> 6
a <<= -3
  a -> -3
```

**Transaction** ([run](https://godbolt.org/z/1T8fG69r8))

```cpp
ureact::context ctx;

ureact::var_signal<int> b = ctx.make_var(1);
ureact::var_signal<int> c = ctx.make_var(1);
ureact::signal<int> a = b + c;

observe(a, []( int a ){ std::cout << "  a -> " << a << "\n"; });

// Normally values are recalculated each change of their dependencies
std::cout << "without transaction\n";
std::cout << "b <<= 2\n";
b <<= 2;
std::cout << "c <<= 2\n";
c <<= 2;

// To perform several changes atomically they should be grouped into a transaction
std::cout << "\nwith transaction\n";
ctx.do_transaction([&]()
{
    std::cout << "b <<= 3\n";
    b <<= 3;
    std::cout << "c <<= 3\n";
    c <<= 3;
});
```

Output:
```
without transaction
b <<= 2
  a -> 3
c <<= 2
  a -> 4

with transaction
b <<= 3
c <<= 3
  a -> 6
```

See other examples at the [examples](tests/src/examples) folder.

## License

<img align="right" src="https://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

The class is licensed under the [MIT License](https://opensource.org/licenses/MIT):

Copyright &copy; 2020 - present [Krylov Yaroslav](https://github.com/YarikTH)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

* * *

This software started as a fork of the [cpp.react](https://github.com/schlangster/cpp.react) by [Sebastian Jeckel](https://github.com/snakster) (schlangster@gmail.com), [BSL 1.0 licensed](http://www.boost.org/LICENSE_1_0.txt).

## Used third-party tools

The library itself consists of a single header file licensed under the MIT license. However, it is built, tested, documented, and whatnot using a lot of third-party tools and services. Thanks a lot!

* [**CMake**](https://cmake.org) for build automation
* [**ClangFormat**](https://clang.llvm.org/docs/ClangFormat.html) for automatic source code indentation
* [**gersemi**](https://github.com/blankspruce/gersemi) for automatic CMake code indentation
* [**doctest**](https://github.com/onqtam/doctest) for the unit tests

## Contact

If you have questions regarding the library, I would like to invite you to [open an issue at GitHub](https://github.com/YarikTH/ureact/issues/new/choose). Please describe your request, problem, or question as detailed as possible, and also mention the version of the library you are using as well as the version of your compiler and operating system. Opening an issue at GitHub allows other users and contributors to this library to collaborate.
