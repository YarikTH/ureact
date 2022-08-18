# Reference

This reference intended to plan a clean and clear interface of a dream.
Descriptions of concrete classes are not the subject of this reference.
Note: this document is not yet contains information about newly added classes and methods

* TODO: rename to Synopsis
* TODO: add information about all new functions and classes

## `var_signal` related

### Creation

Constructor version

```cpp
var_signal<S>::var_signal( context& context, const S& value );
```

Free function version

```cpp
auto make_var( context& context, V&& value ) -> var_signal<auto>;
```

Both versions are functionally equivalent.

### Reading

Get value method (inherited from the signal)

```cpp
const S& var_signal<S>::get() const;
```

Get value operator (inherited from the signal)

```cpp
const S& var_signal<S>::operator()() const;
```

### Modification

Set value method

```cpp
void var_signal<S>::set( const S& new_value ) const;
void var_signal<S>::set( S&& new_value ) const;
```

Set value operator

```cpp
void var_signal<S>::operator<<=( const S& new_value ) const;
void var_signal<S>::operator<<=( S&& new_value ) const;
```

Both versions (method and operator) are functionally equivalent.

Modify method

```cpp
void var_signal<S>::modify( const F& func ) const;
```

Modify operator

```cpp
void var_signal<S>::operator<<=( const F& func ) const;
```

Both versions (method and operator) are functionally equivalent.

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
bool observer::is_valid() const;
bool scoped_observer::is_valid() const;
```

## `signal_pack` related

### Creation

Free function to construct signal_pack

```cpp
auto with( const signal<values_t>&... deps ) -> signal_pack<auto>;
```

## `signal` related

### Creation

Free functions to construct signals

```cpp
auto make_const( context& context, V&& value ) -> signal<auto>;
auto lift( const signal<value_t>& arg, in_f&& func ) -> temp_signal<S, op_t>
auto lift( const signal_pack<values_t...>& arg_pack, in_f&& func ) -> temp_signal<S, op_t>
```

Unary operator overloads to create signals from expressions

```cpp
auto operator+( arg_t&& arg ) -> decltype( temp_signal<S, op_t> );
auto operator-( arg_t&& arg ) -> decltype( temp_signal<S, op_t> );
auto operator!( arg_t&& arg ) -> decltype( temp_signal<S, op_t> );
```

Binary operator overloads to create signals from expressions

```cpp
auto operator+ ( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator- ( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator* ( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator/ ( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator% ( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator==( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator!=( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator< ( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator<=( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator> ( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator>=( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator&&( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
auto operator||( lhs_t&& lhs, rhs_t&& rhs ) -> decltype( temp_signal<S, op_t> );
```

### Reading

Get value method

```cpp
const S& signal<S>::get() const;
```

Get value operator

```cpp
const S& signal<S>::operator()() const;
```

### Other

Tests if this instance is linked to a node

```cpp
bool signal<S>::is_valid() const;
```

## Miscellaneous

Perform several changes atomically

```cpp
auto do_transaction( context& ctx, F&& func, Args&&... args );
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
