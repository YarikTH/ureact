# Changelog

All notable changes to this project will be documented in this file. This
project adheres to [Semantic Versioning](http://semver.org/).

## [0.9.0](https://github.com/YarikTH/ureact/releases/tag/0.9.0) (2023-02-12)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.8.1...0.9.0)

Great closure/adaptor rework

- BREAKING! `sink` and `fork` introduced in 0.8.0 are removed because they don't
  fit into the library and lead to a lot of complicated logic to support them
- BREAKING! ureact::closure is renamed and moved in ureact::detail namespace
  library is not ready for such extension point yet
- BREAKING! free functions that create new reactives (signals/events/observes)
  from existing ones also called "algorithms" are replaced with inline constexpr
  instances of special functor-like classes inherited from Adaptor or AdaptorClosure.
  This is the same approach as used in std::ranges and range-v3.
  There are some differences in usage compared with free functions:
    - [Argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl)
      is not working with adaptors, so namespace is required, while before it was optional
    ```C++
    ureact::context ctx;
    ureact::var_signal<int> src = make_var( ctx, 1 );

    // before 0.9.0
    ureact::signal<int> minus_src = lift( src, std::negate<>{} );

    // after 0.9.0
    ureact::signal<int> minus_src = ureact::lift( src, std::negate<>{} );
    ```
    - single argument adaptors like ureact::monitor and ureact::count no longer
      require functional call scopes
    ```C++
    ureact::context ctx;
    ureact::var_signal src = make_var( ctx, 1 );

    // before 0.9.0
    ureact::signal changes = src | ureact::monitor() | ureact::count();

    // after 0.9.0
    ureact::signal changes = src | ureact::monitor | ureact::count;
    ```
    - unlike free functions, adaptors can't have optional template arguments, so
      new adaptors with underscopes are introduced for such cases, because <>
      would be required for default case.
    ```C++
    ureact::context ctx;
    ureact::event_source<int> src = make_source<int>( ctx );

    // before 0.9.0
    ureact::signal<size_t> changes = src | ureact::count();
    ureact::signal<float> changes_f = src | ureact::count<float>();

    // after 0.9.0
    ureact::signal<size_t> changes = src | ureact::count;
    ureact::signal<float> changes_f = src | ureact::count_<float>;
    ```
- BREAKING! yet another great headers rework:
  - `ureact/take_drop.hpp` is separated into
    `ureact/take.hpp` and `ureact/drop.hpp`
  - `ureact/take_drop_while.hpp` is separated into
    `ureact/take_while.hpp` and `ureact/drop_while.hpp`
  - all adaptors are moved into `ureact/adaptor` subdirectory.
    As a result library headers become easier to navigate.
- New adaptor `ureact::once` is introduced.
  Or reintroduced, because it was added before and then removed in 0.6.0.
  It is merely a `ureact::take(1)` overload with more precise name.
- `ureact::signal_pack` is reworked to hold tuple of signals
  instead of tuple of signal references.
  It reduces space for errors and reduces artificial complexity
  when working with signal_pack with a tiny cost of copying shared
  pointers on reactive value construction.

## [0.8.1](https://github.com/YarikTH/ureact/releases/tag/0.8.1) (2023-01-29)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.8.0...0.8.1)

Fix broken CMake installation script after headers rework made in 0.8.0.

## [0.8.0](https://github.com/YarikTH/ureact/releases/tag/0.8.0) (2023-01-29)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.7.0...0.8.0)

Great headers rework

- BREAKING! "include/ureact/ureact.hpp" is separated into several more
  specialized headers.
  Unlike 0.7.0, ureact.hpp is completely replaced with the new headers.
  More fractional includes allow to reduce inclusion cost.
  All headers are guarantied to be self-contained.
  Use textual search to search wanted functionality because code reference is
  not made yet.
- BREAKING! constructors of `var_signal` and `event_source` that allow
  non-default construction
  without `make_var` and `make_source` are removed
- add function object version of `var_signal<S>::set()`.
  It allows to mimic not only getter functions, but also setter functions.

```C++
struct A
{
    void foo(const int& newValue);
    [[nodiscard]] int foo() const;

    var_signal<int> bar;
};

int main()
{
    A test;
    test.foo(42);
    std::cout << test.foo() << "\n";

    test.bar(42);
    std::cout << test.bar() << "\n";
}
```

- add sanity checks for functions passed in non-observer algorithms such as
  `lift`, `process` and `fold`.
  This check works only if `NDEBUG` is not defined and prevents common misuses:
    - creation of nodes from callback (`make_XXX` and algorithms producing new
      signal/events/observer)
    - read/setting/modification of signal value from callback
    - emitting event from callback
- add `transaction` class that is RAII version of `do_transaction()`
- add `member_XXX` classes that restrict reassignment only for Owner class.
  Base idea was taken
  from https://danieldinu.com/observable/getting-started.html#getting-started-with-observable-properties
  `member_signal<Owner, S>`, `member_var_signal<Owner, S>`,
  `member_events<Owner, E>` and `member_event_source<Owner, E>` are added.
  Interfaces `member_events_user`, `member_signal_user` and
  macro `UREACT_USE_MEMBER_SIGNALS`, `UREACT_USE_MEMBER_EVENTS` are added to
  make usage easier.
- add `sink` algorithm that allows to assign value to reactive value in complex expressions
```C++
ureact::context ctx;

ureact::var_signal<int> src = make_var( ctx, 1 );
ureact::signal<int> squared;
ureact::signal<int> minus_squared;
static const auto square = []( auto i ) { return i * i; };

std::ignore =                                  // TODO: remove it once nodiscard done right
    src                                        //
    | ureact::lift( square ) | sink( squared ) //
    | ureact::lift( std::negate<>{} ) | sink( minus_squared );

assert( squared.get() == 1 );
assert( minus_squared.get() == -1 );

src <<= -2;

assert( squared.get() == 4 );
assert( minus_squared.get() == -4 );
```
- add `fork` algorithm that allows to express multiple transformation of the same source
```C++
ureact::context ctx;

auto src = ureact::make_source<int>( ctx );
static const auto negate = []( auto i ) { return -i; };
static const auto x2 = []( auto i ) { return i * 2; };
static const auto square = []( auto i ) { return i * i; };

ureact::events<int> dst1;
ureact::events<int> dst2;
ureact::events<int> dst;

// makes void lambda to check if fork supports it
auto make_transform_to = []( auto func, ureact::events<int>& where ) {
    return [func, &where]( const auto& src ) { //
        std::ignore =                          // TODO: remove it once nodiscard done right
            src | ureact::transform( func ) | ureact::sink( where );
    };
};

dst =                                                       //
    src                                                     //
    | ureact::fork(                                         //
        ureact::transform( negate ) | ureact::sink( dst1 ), //
        make_transform_to( x2, dst2 )                       //
        )
    | ureact::transform( square );

const auto negate_result = ureact::collect<std::vector>( dst1 );
const auto x2_result = ureact::collect<std::vector>( dst2 );
const auto square_result = ureact::collect<std::vector>( dst );

for( int i = 1; i < 4; ++i )
    src << i;

assert( negate_result.get() == std::vector<int>{ -1, -2, -3 } );
assert( x2_result.get() == std::vector<int>{ 2, 4, 6 } );
assert( square_result.get() == std::vector<int>{ 1, 4, 9 } );
```

## [0.7.0](https://github.com/YarikTH/ureact/releases/tag/0.7.0) (2022-11-27)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.6.0...0.7.0)

Great headers rework. µReact is not longer single-header library

- BREAKING! "include/ureact/ureact.hpp" is separated into several more
  specialized headers.
  "single_include/ureact/ureact_amalgamated.hpp" contains all the headers
  content,
  but has no CMake target. Its main purpose is to be used in godbolt snippets.
- `std::invoke()` is used instead of functional call in most of the places
  receiving
  func or predicate, so member access and member function calls can be
  performed.

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
- BREAKING! `ureact::reactive_ptr()` and `ureact::reactive_ref()` are merged
  into new
  `ureact::reactive_ref()` that works as both
- curried version of `ureact::flatten()` is added
- unary version of `ureact::lift()` is optimized to steal `op`
  from `temp_signal<S>&&`
  sources like unary signal operators did.
  unary signal operators are reworked to call unary `ureact::lift()` with
  standard
  unary functors when it is possible (std::negate<> and others)
- binary version of `ureact::lift()` is added in
  form `ureact::lift(lhs, op, rhs)`.
  It is optimized to steal `op` from `temp_signal<S>&&` sources like binary
  signal
  operators did.
  binary signal operators are reworked to call binary `ureact::lift()` with
  standard
  binary functors when it is possible (std::plus<> and others)
- -Wimplicit-int-float-conversion suppression is removed from lift code
- additional sanity assert in `temp_signal::steal_op()` is added
- curried version of `ureact::observe()` is added
- new `ureact::tap()` algorithm is added. It works like `ureact::observe()`,
  but returns received source instead of observer

## [0.6.0](https://github.com/YarikTH/ureact/releases/tag/0.6.0) (2022-08-18)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.5.0...0.6.0)

Great library rework. There are a lot of changes that would be documented and
cleaned up later

- Port events code from cpp.react
- Introduce lots of breaking changes:
    - revert
      renaming `var_signal->value`, `signal->function`, `make_var->make_value`
    - rename `make_signal/make_function` to `lift`
    - replace `operator|` overload for signals and function with `operator|`
      overload
      on signals and partially applied lift function
    - convert `context::do_transaction()` to free function
    - remove `context::make_value()` use free function make_var() instead
    - remove comma operator overload for signal packs. Use free
      function `with()` instead
    - remove reference overloads for `signal` and `var_signal`.
      Use `std::reference<S>` version instead
