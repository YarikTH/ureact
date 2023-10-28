//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_ADAPTOR_HPP
#define UREACT_DETAIL_ADAPTOR_HPP

#include <tuple>
#include <utility>

#include <ureact/detail/defines.hpp>
#include <ureact/utility/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

// Forward
struct adaptor_closure;

template <typename Lhs, typename Rhs>
class pipe;

/*!
 * @brief Return if type is closure
 */
template <typename T>
struct is_closure : std::is_base_of<adaptor_closure, remove_cvref_t<T>>
{};

/*!
 * @brief Helper variable template for closure
 */
template <typename T>
inline constexpr bool is_closure_v = is_closure<T>::value;

/*!
 * @brief Closure objects used for partial application of reactive functions and chaining of algorithms
 *
 *  Closure objects take one reactive object as its only argument and may return a value.
 *  They are callable via the pipe operator: if C is a closure object and
 *  R is a reactive object, these two expressions are equivalent:
 *  * C(R)
 *  * R | C
 *
 *  Two closure objects can be chained by operator| to produce
 *  another closure object: if C and D are closure objects,
 *  then C | D is also a closure object if it is valid.
 *  The effect and validity of the operator() of the result is determined as follows:
 *  given a reactive object R, these two expressions are equivalent:
 *  * R | C | D // (R | C) | D
 *  * R | (C | D)
 *
 * @note similar to https://en.cppreference.com/w/cpp/ranges#Range_adaptor_closure_objects
 */
struct adaptor_closure
{
    /// chain two closures to make another one
    template <typename Lhs,
        typename Rhs,
        class = std::enable_if_t<is_closure_v<Lhs>>,
        class = std::enable_if_t<is_closure_v<Rhs>>>
    UREACT_WARN_UNUSED_RESULT friend constexpr auto operator|( Lhs lhs, Rhs rhs )
    {
        return pipe<Lhs, Rhs>{ std::move( lhs ), std::move( rhs ) };
    }

    /// apply arg to given closure and return its result
    template <typename Self,
        typename Reactive,
        class = std::enable_if_t<is_closure_v<Self>>,
        class = std::enable_if_t<!is_closure_v<Reactive>>,
        class = std::enable_if_t<std::is_invocable_v<Self&&, Reactive&&>>>
    UREACT_WARN_UNUSED_RESULT friend constexpr auto operator|( Reactive&& r, Self&& self )
    {
        return std::forward<Self>( self )( std::forward<Reactive>( r ) );
    }
};

/// Composition of the adaptor closures Lhs and Rhs.
template <typename Lhs, typename Rhs>
class pipe : public adaptor_closure
{
public:
    constexpr pipe( Lhs lhs, Rhs rhs )
        : m_lhs( std::move( lhs ) )
        , m_rhs( std::move( rhs ) )
    {}

    template <typename Reactive>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Reactive&& r ) const&
    {
        return m_rhs( m_lhs( std::forward<Reactive>( r ) ) );
    }

    template <typename Reactive>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Reactive&& r ) &&
    {
        return std::move( m_rhs )( std::move( m_lhs )( std::forward<Reactive>( r ) ) );
    }

    template <typename Reactive>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Reactive&& r ) const&& = delete;

private:
    Lhs m_lhs;
    Rhs m_rhs;
};

/// Partial application of the adaptor
template <typename Adaptor, typename... Args>
class partial : public adaptor_closure
{
public:
    constexpr explicit partial( Args... args )
        : m_args( std::move( args )... )
    {}

    template <typename Reactive>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Reactive&& r ) const&
    {
        auto forwarder = [&r]( const auto&... args ) {
            return Adaptor{}( std::forward<Reactive>( r ), args... );
        };
        return std::apply( forwarder, m_args );
    }

    template <typename Reactive>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Reactive&& r ) &&
    {
        auto forwarder = [&r]( auto&... args ) {
            return Adaptor{}( std::forward<Reactive>( r ), std::move( args )... );
        };
        return std::apply( forwarder, m_args );
    }

    template <typename Reactive>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Reactive&& r ) const&& = delete;

private:
    std::tuple<Args...> m_args;
};

/*!
 * @brief Base class for reactive adaptors
 *
 *  Even if it is empty, inheritance allows to find each adaptor in the library
 *  
 *  Equivalent of "Range adaptors" from std ranges library
 */
struct adaptor
{
protected:
    template <typename Derived, typename... Args>
    static constexpr auto make_partial( Args&&... args )
    {
        return partial<Derived, std::decay_t<Args>...>{ std::forward<Args>( args )... };
    }
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_ADAPTOR_HPP
