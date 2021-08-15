# Reference

This reference intended to plan a clean and clear interface of a dream.
Descriptions of concrete classes are not the subject of this reference.

## `var_signal` related

* TODO: rename `var_signal` to `value`
* TODO: remove inheritance from `signal<S>`, replace with inheritance from `signal_base<S>` or something 
* TODO: modify method for `var_signal<S&>`?
* TODO: operator version of `var_signal::modify()`?
* TODO: functional version of `var_signal::value()`?

### Creation

Free function version

```cpp
template<typename V>
auto make_var( context& context, V&& value ) -> var_signal<auto>;
```

Member function version

```cpp
template<typename V>
auto context::make_var( V&& value ) -> var_signal<auto>;
```

Both versions are functionally equivalent.

### Reading

Get value method (inherited from the signal)

```cpp
const S& var_signal<S>::value() const;
const S& var_signal<S&>::value() const;
```

### Modification

Set value method

```cpp
void var_signal<S>::set( const S& new_value ) const;
void var_signal<S>::set( S&& new_value ) const;
void var_signal<S&>::set( std::reference_wrapper<S> new_value ) const;
```

Set value operator

```cpp
void var_signal<S>::operator<<=( const S& new_value ) const;
void var_signal<S>::operator<<=( S&& new_value ) const;
void var_signal<S&>::operator<<=( std::reference_wrapper<S> new_value ) const;
```

Both versions (method and operator) are functionally equivalent.

Modify method

```cpp
template<typename F>
void var_signal<S>::modify( const F& func ) const;
```

### Other

Tests if this instance is linked to a node

```cpp
bool var_signal<S>::is_valid() const;
```

## `observer` related

### Creation

Free function to construct observers

```cpp
// in_f is either void(const S&) or observer_action(const S&)
template <typename in_f, typename S>
auto observe( const signal<S>& subject, in_f&& func ) -> observer;
```

Move constructor to create a scoped_observer from an observer

```cpp
scoped_observer::scoped_observer( observer&& obs );
```

### Other

Method to manually detach observer

```cpp
void observer::detach();
```

Tests if this instance is linked to a node

```cpp
bool observer::is_valid () const;
bool scoped_observer::is_valid () const;
```

## `signal_pack` related

### Creation

Free function to construct signal_pack

```cpp
auto with( const signal<values_t>&... deps ) -> signal_pack<auto>;
```

Operator overload to construct signal_pack

```cpp
auto operator,( const signal<left_val_t>& a, const signal<right_val_t>& b ) -> signal_pack<auto>;
auto operator,( const signal_pack<cur_values_t...>& cur, const signal<append_value_t>& append ) -> signal_pack<auto>;
```

Both versions are functionally equivalent

## `signal` related

* TODO: rename `signal` to `function`
* TODO: functional version of `signal::value()`?
* TODO: convert `operator|` to return `detail::temp_signal<S, op_t>`

### Creation

Note: all creation functions return `detail::temp_signal<S, op_t>` which inherited from
`signal<S>` so `signal<S>` are constructable from `detail::temp_signal<S, op_t>`.

Free function to construct signals

```cpp
auto make_signal( const signal<value_t>& arg, in_f&& func ) -> detail::temp_signal<S, op_t>
auto make_signal( const signal_pack<values_t...>& arg_pack, in_f&& func ) -> detail::temp_signal<S, op_t>
```

Operator overload to construct signals

```cpp
auto operator|( const signal_t<value_t>& arg, F&& func ) -> signal<auto>
auto operator|( const signal_pack<values_t...>& arg_pack, F&& func ) -> signal<auto>
```

Deprecated operator overload to construct signals

```cpp
auto operator->*( const signal_t<value_t>& arg, F&& func ) -> signal<auto>
auto operator->*( const signal_pack<values_t...>& arg_pack, F&& func ) -> signal<auto>
```

Unary operator overloads to create signals from expressions

```cpp
auto operator+(arg_t&& arg) -> decltype(detail::temp_signal<S, op_t>)
auto operator-(arg_t&& arg) -> decltype(detail::temp_signal<S, op_t>)
auto operator!(arg_t&& arg) -> decltype(detail::temp_signal<S, op_t>)
```

Binary operator overloads to create signals from expressions

```cpp
auto operator+ (lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator- (lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator* (lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator/ (lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator% (lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator==(lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator!=(lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator< (lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator<=(lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator> (lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator>=(lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator&&(lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
auto operator||(lhs_t&& lhs, rhs_t&& rhs) -> decltype(detail::temp_signal<S, op_t>);
```

Note: bitwise operator overloads were removed in 0.3.0 without deprecation. List of removed overloads: `~`, `&`, `|`, `^`, `<<`, `>>`.

### Reading

Get value method

```cpp
const S& signal<S>::value() const;
const S& signal<S&>::value() const;
```

### Other

Tests if this instance is linked to a node

```cpp
bool signal<S>::is_valid() const;
```

## Miscellaneous

Perform several changes atomically

```cpp
template<typename F>
void context::do_transaction( F&& func );
```

Return if the type is signal or its inheritor

```cpp
template<class T>
struct is_signal;
```

Algorithm to create a signal that points to the inner signal's value.

```cpp
auto flatten( const signal<signal<inner_value_t>>& outer ) -> signal<inner_value_t>;
```


---------------

[Home](readme.md#reference)
