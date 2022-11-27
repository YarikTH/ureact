# Changelog

All notable changes to this project will be documented in this file. This project adheres to [Semantic Versioning](http://semver.org/).

## [0.6.0](https://github.com/YarikTH/ureact/releases/tag/0.6.0) (2022-08-18)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.5.0...0.6.0)

Great library rework. There are a lot of changes that would be documented and cleaned up later

- Port events code from cpp.react
- Introduce lots of breaking changes:
  - revert renaming `var_signal->value`, `signal->function`, `make_var->make_value`
  - rename `make_signal/make_function` to `lift`
  - replace `operator|` overload for signals and function with `operator|` overload
    on signals and partially applied lift function
  - convert `context::do_transaction()` to free function
  - remove `context::make_value()` use free function make_var() instead
  - remove comma operator overload for signal packs. Use free function `with()` instead
  - remove reference overloads for `signal` and `var_signal`. Use `std::reference<S>` version instead

## [0.7.0](https://github.com/YarikTH/ureact/releases/tag/0.7.0) (2022-11-27)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.6.0...0.7.0)

Great headers rework. µReact is not longer single-header library

  - "include/ureact/ureact.hpp" is separated into several more specialized headers.
    "single_include/ureact/ureact_amalgamated.hpp" contains all the headers content,
    but has no CMake target. Its main purpose is to be used in godbolt snippets.
  - `std::invoke()` is used instead of functional call in most of the places receiving
    func or predicate, so member access and member function calls can be performed.
```C++
struct User
{
    int age;
    int get_age(){ return age; }
};

ureact::context ctx;
auto user = make_var( ctx, User{18});
auto user_age = lift( user, &User::age );
assert( user_age.get() == 18 );
auto user_age_2 = lift( user, &User::get_age );
assert( user_age_2.get() == 18 );
```
  - `ureact::lift()` overloads have optional first template parameter to select
    desired type of resulting signal
  - default signal type deduction for `ureact::lift()` removes references from
    the invoke result type
  - `ureact::reactive_ptr()` and `ureact::reactive_ref()` are merged into new
    `ureact::reactive_ref()` that works as both
  - curried version of `ureact::flatten()` is added
  - unary version of `ureact::lift()` is optimized to steal `op` from `temp_signal<S>&&`
    sources like unary signal operators did.
    unary signal operators are reworked to call unary `ureact::lift()` with standard
    unary functors when it is possible (std::negate<> and others)
  - binary version of `ureact::lift()` is added in form `ureact::lift(lhs, op, rhs)`.
    It is optimized to steal `op` from `temp_signal<S>&&` sources like binary signal
    operators did.
    binary signal operators are reworked to call binary `ureact::lift()` with standard
    binary functors when it is possible (std::plus<> and others)
  - -Wimplicit-int-float-conversion suppression is removed from lift code
  - additional sanity assert in `temp_signal::steal_op()` is added
  - curried version of `ureact::observe()` is added
  - new `ureact::tap()` algorithm is added. It works like `ureact::observe()`,
    but returns received source instead of observer
