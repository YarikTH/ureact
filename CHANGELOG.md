# Changelog

All notable changes to this project will be documented in this file. This project adheres to [Semantic Versioning](http://semver.org/).

## [0.6.0](https://github.com/YarikTH/ureact/releases/tag/0.6.0) (2022-08-18)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.5.0...0.6.0)

Great library rework. There are a lot of changes that would be documented and cleaned up later

- Port events code from cpp.react
- Introduce lots of breaking changes:
  - revert renaming var_signal->value, signal->function, make_var->make_value
  - rename make_signal/make_function to lift
  - replace operator| overload for signals and function with operator| overload on signals and partially applied lift function
  - convert context::do_transaction() to free function
  - remove context::make_value() use free function make_var() instead
  - remove comma operator overload for signal packs. Use free function with() instead
  - remove reference overloads for signal and var_signal. Use std::reference<S> version instead

