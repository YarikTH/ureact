//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_TYPE_TRAITS_HPP
#define UREACT_TYPE_TRAITS_HPP

#include <type_traits>

#include <ureact/defines.hpp>

UREACT_BEGIN_NAMESPACE

// Got from https://stackoverflow.com/a/34672753
// std::is_base_of for template classes
namespace detail
{

template <template <typename...> class Base, typename Derived>
struct is_base_of_template_impl
{
    template <typename... Ts>
    static constexpr std::true_type test( const Base<Ts...>* )
    {
        return {};
    }
    static constexpr std::false_type test( ... )
    {
        return {};
    }
    using type = decltype( test( std::declval<Derived*>() ) );
};

template <template <typename...> class Base, typename Derived>
using is_base_of_template = typename is_base_of_template_impl<Base, Derived>::type;

} // namespace detail


// Forward
template <typename S>
class signal;

/*!
 * @brief Return if type is signal or its inheritor
 */
template <typename T>
struct is_signal : detail::is_base_of_template<signal, T>
{};

/*!
 * @brief Helper variable template for is_signal
 */
template <typename T>
inline constexpr bool is_signal_v = is_signal<T>::value;



// Forward
template <typename S>
class var_signal;

/*!
 * @brief Return if type is var_signal or its inheritor
 */
template <typename T>
struct is_var_signal : detail::is_base_of_template<var_signal, T>
{};

/*!
 * @brief Helper variable template for is_var_signal
 */
template <typename T>
inline constexpr bool is_var_signal_v = is_var_signal<T>::value;



// Forward
template <typename... Values>
class signal_pack;

/*!
 * @brief Return if type is signal_pack
 */
template <typename T>
struct is_signal_pack : detail::is_base_of_template<signal_pack, T>
{};

/*!
 * @brief Helper variable template for is_signal_pack
 */
template <typename T>
inline constexpr bool is_signal_pack_v = is_signal_pack<T>::value;



/*!
 * @brief Return if type is signal's inheritor or signal_pack
 */
template <typename T>
struct is_signal_or_pack : std::disjunction<is_signal<T>, is_signal_pack<T>>
{};

/*!
 * @brief Helper variable template for is_signal_or_pack
 */
template <typename T>
inline constexpr bool is_signal_or_pack_v = is_signal_or_pack<T>::value;



// Forward
template <typename E>
class events;

/*!
 * @brief Return if type is events or its inheritor
 */
template <typename T>
struct is_event : detail::is_base_of_template<events, T>
{};

/*!
 * @brief Helper variable template for is_event
 */
template <typename T>
inline constexpr bool is_event_v = is_event<T>::value;



// Forward
template <typename E>
class event_source;

/*!
 * @brief Return if type is event_source or its inheritor
 */
template <typename T>
struct is_event_source : detail::is_base_of_template<event_source, T>
{};

/*!
 * @brief Helper variable template for is_event_source
 */
template <typename T>
inline constexpr bool is_event_source_v = is_event_source<T>::value;



// Forward
class observer;

/*!
 * @brief Return if type is observer
 */
template <typename T>
struct is_observer : std::is_same<T, observer>
{};

/*!
 * @brief Helper variable template for is_observer
 */
template <typename T>
inline constexpr bool is_observer_v = is_observer<T>::value;



/*!
 * @brief Return if type is signal or event inheritor
 */
template <typename T>
struct is_observable : std::disjunction<is_signal<T>, is_event<T>>
{};

/*!
 * @brief Helper variable template for is_observable
 */
template <typename T>
inline constexpr bool is_observable_v = is_observable<T>::value;



/*!
 * @brief Return if type is signal or event or observer
 */
template <typename T>
struct is_reactive : std::disjunction<is_observable<T>, is_observer<T>>
{};

/*!
 * @brief Helper variable template for is_reactive
 */
template <typename T>
inline constexpr bool is_reactive_v = is_reactive<T>::value;



// Forward
template <class F>
class closure;

/*!
 * @brief Return if type is closure
 */
template <typename T>
struct is_closure : detail::is_base_of_template<closure, T>
{};

/*!
 * @brief Helper variable template for closure
 */
template <typename T>
inline constexpr bool is_closure_v = is_closure<T>::value;


UREACT_END_NAMESPACE

#endif //UREACT_TYPE_TRAITS_HPP
