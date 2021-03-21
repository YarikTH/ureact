#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

namespace ureact
{
namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper to enable calling a function on each element of an argument pack.
/// We can't do f(args) ...; because ... expands with a comma.
/// But we can do nop_func(f(args) ...);
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... args_t>
inline void pass( args_t&&... /*unused*/ )
{}

///////////////////////////////////////////////////////////////////////////////////////////////////
// identity (workaround to enable enable decltype()::X)
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct identity
{
    using type = T;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// dont_move!
///////////////////////////////////////////////////////////////////////////////////////////////////
struct dont_move
{};

template <typename T1, typename T2>
using is_same_decay = std::is_same<typename std::decay<T1>::type, typename std::decay<T2>::type>;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// add_default_return_value_wrapper
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename F, typename ret_t, ret_t return_value>
struct add_default_return_value_wrapper
{
    add_default_return_value_wrapper( const add_default_return_value_wrapper& ) = default;

    add_default_return_value_wrapper& operator=( const add_default_return_value_wrapper& ) = delete;

    add_default_return_value_wrapper( add_default_return_value_wrapper&& other ) noexcept
        : my_func( std::move( other.my_func ) )
    {}

    add_default_return_value_wrapper& operator=(
        add_default_return_value_wrapper&& ) noexcept = delete;

    template <typename in_f,
        class = typename std::enable_if<
            !is_same_decay<in_f, add_default_return_value_wrapper>::value>::type>
    explicit add_default_return_value_wrapper( in_f&& func )
        : my_func( std::forward<in_f>( func ) )
    {}

    ~add_default_return_value_wrapper() = default;

    template <typename... args_t>
    ret_t operator()( args_t&&... args )
    {
        my_func( std::forward<args_t>( args )... );
        return return_value;
    }

    F my_func;
};

// Expand args by wrapping them in a dummy function
// Use comma operator to replace potential void return value with 0
#define REACT_EXPAND_PACK( ... ) ::ureact::detail::pass( ( __VA_ARGS__, 0 )... )

} // namespace detail
} // namespace ureact
