//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_CONTAINER_TYPE_TRAITS_HPP
#define UREACT_DETAIL_CONTAINER_TYPE_TRAITS_HPP

#include <tuple>
#include <type_traits>
#include <vector> // for std::begin(). It is declared in core headers anyway

#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename Cont, typename Value, typename = void>
struct has_push_back_method : std::false_type
{};

template <typename Cont, class Value>
struct has_push_back_method<Cont,
    Value,
    std::void_t<decltype( std::declval<Cont>().push_back( std::declval<Value>() ) )>>
    : std::true_type
{};

template <typename Cont, class Value>
inline constexpr bool has_push_back_method_v = has_push_back_method<Cont, Value>::value;

template <typename Cont, typename Value, typename = void>
struct has_insert_method : std::false_type
{};

template <typename Cont, class Value>
struct has_insert_method<Cont,
    Value,
    std::void_t<decltype( std::declval<Cont>().insert( std::declval<Value>() ) )>> : std::true_type
{};

template <typename Cont, class Value>
inline constexpr bool has_insert_method_v = has_insert_method<Cont, Value>::value;

template <typename Cont, typename Key, typename Value, typename = void>
struct has_array_subscript_operator : std::false_type
{};

template <typename Cont, typename Key, typename Value>
struct has_array_subscript_operator<Cont,
    Key,
    Value,
    std::void_t<decltype( std::declval<Cont>()[std::declval<Key>()] = std::declval<Value>() )>>
    : std::true_type
{};

template <typename Cont, typename Key, typename Value>
inline constexpr bool has_array_subscript_operator_v
    = has_array_subscript_operator<Cont, Key, Value>::value;

template <class ContainerT>
struct container_value
{
    using type = std::decay_t<decltype( *std::begin( std::declval<ContainerT>() ) )>;
};

template <class ContainerT>
using container_value_t = typename container_value<ContainerT>::type;

template <template <typename...> class Cont, typename Value, typename = void>
struct is_sequence_container : std::false_type
{};

template <template <typename...> class Cont, class Value>
struct is_sequence_container<Cont, Value, std::void_t<decltype( std::declval<Cont<Value>>() )>>
    : std::true_type
{};

template <template <typename...> class Cont, class Value>
inline constexpr bool is_sequence_container_v = is_sequence_container<Cont, Value>::value;

template <template <typename...> class Cont, typename Value, typename = void>
struct is_associative_container : std::false_type
{};

template <template <typename...> class Cont, class Value>
struct is_associative_container<Cont,
    Value,
    std::void_t<decltype( std::declval<
        Cont<std::tuple_element_t<0, Value>, std::tuple_element_t<1, Value>>>() )>> : std::true_type
{};

template <template <typename...> class Cont, class Value>
inline constexpr bool is_associative_container_v = is_associative_container<Cont, Value>::value;

template <template <typename...> class Cont, typename Value, typename = void>
struct container_detector
{
    using type = void;
};

template <template <typename...> class Cont, class Value>
struct container_detector<Cont, Value, std::enable_if_t<is_sequence_container_v<Cont, Value>>>
{
    using type = Cont<Value>;
};

template <template <typename...> class Cont, class Value>
struct container_detector<Cont,
    Value,
    std::enable_if_t<
        is_associative_container_v<Cont, Value> && !is_sequence_container_v<Cont, Value>>>
{
    using type = Cont<std::tuple_element_t<0, Value>, std::tuple_element_t<1, Value>>;
};

template <template <typename...> class Cont, class Value>
using container_detector_t = typename container_detector<Cont, Value>::type;

} // namespace detail

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_CONTAINER_TYPE_TRAITS_HPP
