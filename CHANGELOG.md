# Changelog

All notable changes to this project will be documented in this file. This
project adheres to [Semantic Versioning](http://semver.org/).

## [0.16.0](https://github.com/YarikTH/ureact/releases/tag/0.16.0) (2023-09-10)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.15.0...0.16.0)

- BREAKING! #126 Improve compile times
    - Inclusion of `<functional>` and `<iterator>` hurts compilation times, so
      their usage is removed for core headers (all, but adaptor).
    - allow only functors in var_signal::modify() (any invokable were valid
      before). It allows not including `<functional>` in `<ureact/signal.hpp>`
- #134 Get rid of issue_template.md
- #136 Work around broken `ureact::merge` with latest MSVC in c++20 mode
- #133 Fix MSVC's /W4 warnings

## [0.15.0](https://github.com/YarikTH/ureact/releases/tag/0.15.0) (2023-08-18)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.14.0...0.15.0)

- BREAKING! Remove `do_transaction()` function.  
  It is literally a function receiving a function and calling it in the scope
  with `transaction`
- BREAKING! Rename `default_context::transaction`
  to `default_context::default_transaction`  
  `transaction _;` won't longer be ambiguous when both `ureact`
  and `ureact::default_context` namespaces are used
- #121 Add `transaction::finish()` to finish transaction before code scope is
  ended
- #127 Get rid of `ureact_amalgamated.hpp`  
  It was used for godbolt example only, but now `µReact` is added on godbolt, so
  there is no longer reason to have it
- Allow calling `zip` with a single argument
- Add `zip_transform` adaptor
- Add `enumerate` adaptor
- Add `adjustment` and `pairwise` adapters of all possible flavors:
    - `adjacent`
    - `adjacent_filter`
    - `adjacent_transform`
    - `pairwise`
    - `pairwise_filter`
    - `pairwise_transform`
- #111 Add associative containers support in `collect` adaptor
- #122 Support iterators as `observer`'s argument
- #123 Make sure transactions can't be started or finished when turn is in
  progress
- Fix `member_` by making their copy/move constructors/assignments private

## [0.14.0](https://github.com/YarikTH/ureact/releases/tag/0.14.0) (2023-07-16)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.13.0...0.14.0)

Rework header structure and rework observers

- Add `ureact::happened` adaptor
- BREAKING! Big structure update \
  Root `ureact` folder become clearer (6 headers vs 14) and `ureact/adaptor`
  folder now mostly contain only so named adaptors in each header.
    - move `ureact/default_context.hpp` content into `ureact/context.hpp`
      it contains a single function, and it is unconditionally included
      in `ureact/signal.hpp` and `ureact/events.hpp` anyway
    - move `ureact/temp_signal.hpp` into `ureact/detail`
      because it is not intended to be included and used by library user
    - move `ureact/has_changed.hpp` into `ureact/detail`
      because it is not intended to be included and used by library user. User
      can define its own `has_changed` overload, but `has_changed.hpp` is not
      needed to do so.
    - move some headers that are not useful by themselves in a
      new `ureact/utility` directory. Namely:
        - `ureact/unit.hpp`
        - `ureact/type_traits.hpp`
        - `ureact/signal_pack.hpp`
        - `ureact/event_range.hpp`
        - `ureact/event_emitter.hpp`
    - extract `ureact::join_with` from `ureact/adaptor/join.hpp`
      to `ureact/adaptor/join_with.hpp`
    - extract `ureact::changed_to` from `ureact/adaptor/changed.hpp`
      to `ureact/adaptor/changed_to.hpp`
    - extract `ureact::keys` and `ureact::values`
      from `ureact/adaptor/elements.hpp`
      to `ureact/adaptor/keys.hpp` and `ureact/adaptor/values.hpp`
- BREAKING! #120 Rework observers
    - observable_node is no longer owns observers, and it is removed at all (
      node_base is used instead)
    - observer nodes own their subjects
    - scoped_observer is removed
    - observer owns its observer node and nothing else
    - tap nodes are introduced to own both observer and observed node
    - result of observe should no longer discarded for both L-value and R-value
      subjects
    - See #120 for examples and problem description
- #113 Add observe_policy, so even current signal values can be notified if
  needed

## [0.13.0](https://github.com/YarikTH/ureact/releases/tag/0.13.0) (2023-06-17)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.12.0...0.13.0)

- Add initial version of code coverage collection
- Improve tests
- Make `ureact::transform` compatible with invokable that return reference Now
  it works the same way `ureact::lift` does - invoke result is decayed to deduce
  result `ureact::events<E>` type
- Add `ureact::transform_as` to manually select result type instead of relying
  on the deduced type

## [0.12.0](https://github.com/YarikTH/ureact/releases/tag/0.12.0) (2023-05-09)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.11.0...0.12.0)

Rework "has_changed" extension point and replace doctest with Cache2

- BREAKING! rework approach to "calming" made via has_changed customization
  point
  Overloaded free function version of `ureact::detail::has_changed()` is
  replaced with Customization Point Object
  Existence of operator== is no longer condition for "calming", instead free
  function `has_changed` is used.
  `has_changed` is provided only for arithmetic types, enum types and library
  classes `ureact::signal` and `ureact::events`.

  Examples of `has_changed` function:
    1. free function in the same namespace
    ```C++
    namespace foo
    {
        struct Foo
        {
            int value;
        };
        
        constexpr bool has_changed( const Foo lhs, const Foo rhs ) noexcept
        {
            return !( lhs.value == rhs.value );
        }
    }
    ```

    2. hidden friend version for the shy ones
    ```C++
    namespace bar
    {
        class Bar
        {
            int value;
           
            friend constexpr bool has_changed( const Bar lhs, const Bar rhs ) noexcept
            {
                return !( lhs.value == rhs.value );
            }
        };
    }
    ```
- Remove Codacy badge. Codacy proved to be useless for this repository
- Replace doctest with Cache2. They are mostly the same in terms of interface,
  but activity and support of Cache2 is better
- Move the only benchmark from standalone benchmark folder into ureact_test
  target.
  Cache2 has microbenchmark feature, but I don't understand it's output and
  don't trust it.

## [0.11.0](https://github.com/YarikTH/ureact/releases/tag/0.11.0) (2023-05-01)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.10.1...0.11.0)

Make library usage more safe to use and add `default_context` feature

- Rework internal objects' lifetime safety
- Add protection against using nodes from different contexts
- Fix bug where synced `event_range` based observer callbacks were called
  without events
  if any of signals in dep_pack are changed. It contradicts to a note:
  "Changes of signals in dep_pack do not trigger an update - only received
  events do"
- BREAKING! Change order of dep_pack... args passed in functor passed in process
  adaptor
  ```C++
  ureact::context ctx;
  
  using record_t = std::pair<std::string, int>;
  
  auto src = make_source<int>( ctx );
  auto n = make_var<unsigned>( ctx, {} );
  auto timestamp = make_var<std::string>( ctx, {} );
  ureact::events<record_t> processed;
  
  // Before. Arguments corresponding to dep_pack are placed in the end of argument list
  const auto repeater = []( ureact::event_range<int> range,          // src
                            ureact::event_emitter<record_t> out,
                            unsigned n, const std::string& timestamp // with( n, timestamp )
                          ) {
  };
  
  // After. Arguments corresponding to dep_pack are placed just after event_range,
  // that corresponds to the order of arguments passed to "ureact::process"
  // and matches approach of "synced" versions of other adaptors
  const auto repeater = []( ureact::event_range<int> range,           // src
                            unsigned n, const std::string& timestamp, // with( n, timestamp )
                            ureact::event_emitter<record_t> out
                          ) {
      ...
  };
  
  processed = ureact::process<record_t>( src, with( n, timestamp ), repeater );
  ```
- Add default context feature. It is more example friendly and maybe more
  user-friendly in general.
  This feature adds a new namespace `ureact::default_context` and several new
  functions and classes there:
    - function `default_context::get()` in `ureact/default_context.hpp`
    - functions `default_context::make_var()`
      and `default_context::make_const()` in `ureact/signal.hpp`
    - functions `default_context::make_source()`
      and `default_context::make_never()` in `ureact/events.hpp`
    - class `default_context::transaction` and
      function `default_context::do_transaction()` in `ureact/transaction.hpp`

  `default_context::get()` returns thread_local copy of default context if it
  exists or makes a new one.
  It is mostly intended to be used by other functions and classes
  of `default_context` instead of direct usage.

  All other functions and classes are just strait up analogs of entities
  outside `default_context`,
  the only difference - they don't have `context` arguments, instead they
  use `context` received from `default_context::get()`.

  Without `default_context` feature:
  ```C++
  ureact::context ctx;
  
  auto var = ureact::make_var( ctx, 1 );
  auto cst = ureact::make_const( ctx, 2 );
  auto src = ureact::make_source<int>( ctx );
  auto nvr = ureact::make_never<int>( ctx );
  
  do_transaction( ctx, [&](){
      ureact::transaction _( ctx );
      ...
  });
  ```

  With `default_context` feature:
  ```C++
  using namespace ureact::default_context;
  
  auto var = make_var( 1 );
  auto cst = make_const( 2 );
  auto src = make_source<int>();
  auto nvr = make_never<int>();
  
  do_transaction( [&](){
      transaction _;
      ...
  });
  ```

## [0.10.1](https://github.com/YarikTH/ureact/releases/tag/0.10.1) (2023-03-04)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.10.0...0.10.1)

Upgrade CI scripts and fix code to work with old gcc-7 and clang-6 compilers.

- Improve CI scripts for Linux.
  Move to Ubuntu 22.04 because 20.04 image will be removed soon.
  Add linux build configurations in `[gcc-7; gcc-11]` and `[clang-6; clang-11]`
  ranges.
- Fix build for older compilators. In particular by removing explicit `noexcept`
  specifications for move constructors.
  Older compilators don't mark all used std classes as nothrow movable, so it
  conflicts with explicit `noexcept` specification from ureact.

## [0.10.0](https://github.com/YarikTH/ureact/releases/tag/0.10.0) (2023-02-27)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.9.0...0.10.0)

Adapting `context` ownership scheme from master branch of cpp.react

- BREAKING! `context` is no longer a thing that user should guarantee to die
  last
  after each ureact nodes using this `context` are destroyed.
  Instead, now it has shared pointer semantic and each ureact node has a copy of
  it.
  So `context` dies only after all its owners diy.
- BREAKING! `count_` and `lift_` are renamed to `count_as` and `lift_as`
  respectively.
- BREAKING! `merge` adaptor is no longer selecting result type based on the type
  of
  the first source event stream, but tries to determine result type
  using `std::common_type`.
  Additionally `merge_as` version is added to manually select the result type.
- BREAKING! `equal_to` is replaced with the opposite `has_changed`.
  All user overloads of `equal_to` are impacted.
- New adaptor `monitor_change` is added. It emits (old value, new value) pairs
  on
  monitored signal changes.
- New adaptor `elements` and its specializations `keys` and `values` are added.
  They do exactly the same that analogs from `std::ranges` do.
  https://en.cppreference.com/w/cpp/ranges/elements_view
- New adaptor `stride` is added.
  It does exactly the same that analog from `std::ranges` does.
  https://en.cppreference.com/w/cpp/ranges/stride_view
- New adaptor `slice` is added.
  `ureact::slice(M, N)` is equivalent
  to `ureact::drop(M) | ureact::take(N - M)`.

## [0.9.0](https://github.com/YarikTH/ureact/releases/tag/0.9.0) (2023-02-12)

[Full Changelog](https://github.com/YarikTH/ureact/compare/0.8.1...0.9.0)

Great closure/adaptor rework

- BREAKING! `sink` and `fork` introduced in 0.8.0 are removed because they don't
  fit into the library and lead to a lot of complicated logic to support them
- BREAKING! ureact::closure is renamed and moved in ureact::detail namespace
  library is not ready for such extension point yet
- BREAKING! free functions that create new reactives (signals/events/observes)
  from existing ones also called "algorithms" are replaced with inline constexpr
  instances of special functor-like classes inherited from Adaptor or
  AdaptorClosure.
  This is the same approach as used in std::ranges and range-v3.
  There are some differences in usage compared with free functions:
    - [Argument-dependent lookup](https://en.cppreference.com/w/cpp/language/adl)
      is not working with adaptors, so namespace is required, while before it
      was optional
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
- add `sink` algorithm that allows to assign value to reactive value in complex
  expressions

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

- add `fork` algorithm that allows to express multiple transformation of the
  same source

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
