#pragma once

namespace ureact
{

// Got from https://stackoverflow.com/a/34672753
// std::is_base_of for template classes
template <template <typename...> class base, typename derived>
struct is_base_of_template_impl
{
    template <typename... Ts>
    static constexpr std::true_type test( const base<Ts...>* );
    static constexpr std::false_type test( ... );
    using type = decltype( test( std::declval<derived*>() ) );
};

template <template <typename...> class base, typename derived>
using is_base_of_template = typename is_base_of_template_impl<base, derived>::type;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// is_signal
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class signal;

template <typename T>
struct is_signal : is_base_of_template<signal, T>
{};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// decay_input
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class var_signal;

template <typename T>
struct decay_input
{
    using type = T;
};

template <typename T>
struct decay_input<var_signal<T>>
{
    using type = signal<T>;
};

} // namespace ureact
