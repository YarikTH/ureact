//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_CONTAINER_TYPE_TRAITS_HPP
#define UREACT_DETAIL_CONTAINER_TYPE_TRAITS_HPP

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

template <class ContainerT>
struct container_value
{
    using type = std::decay_t<decltype( *std::begin( std::declval<ContainerT>() ) )>;
};

template <class ContainerT>
using container_value_t = typename container_value<ContainerT>::type;

} // namespace detail

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_CONTAINER_TYPE_TRAITS_HPP
