//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_CLOSURE_HPP
#define UREACT_CLOSURE_HPP

#include <ureact/type_traits.hpp>
#include <ureact/ureact.hpp>

UREACT_BEGIN_NAMESPACE

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
template <class F>
class closure
{
public:
    // TODO: add type specialization to closure (not concrete type, but signal/events), so we can write specialized operator overloads
    //       need to tag both input and output value. Also closure is single in, single out function. Or no output function
    explicit closure( F&& func )
        : m_func( std::move( func ) )
    {}

    template <typename Arg, class = std::enable_if_t<std::is_invocable_v<F, Arg&&>>>
    UREACT_WARN_UNUSED_RESULT auto operator()( Arg&& arg ) const -> decltype( auto )
    {
        if constexpr( std::is_same_v<std::invoke_result_t<F, Arg&&>, void> )
        {
            m_func( std::forward<Arg>( arg ) );
        }
        else
        {
            return m_func( std::forward<Arg>( arg ) );
        }
    }

private:
    F m_func;
};

/*!
 * @brief operator| overload for closure object
 *
 *  See @ref closure
 */
template <typename Arg,
    typename Closure,
    class = std::enable_if_t<is_closure_v<std::decay_t<Closure>>>>
UREACT_WARN_UNUSED_RESULT auto operator|( Arg&& arg, Closure&& closure_obj ) -> decltype( auto )
{
    if constexpr( is_closure_v<std::decay_t<Arg>> )
    {
        // chain two closures to make another one
        using FirstClosure = Arg;
        using SecondClosure = Closure;
        return closure{
            [first_closure = std::forward<FirstClosure>( arg ),
                second_closure = std::forward<SecondClosure>( closure_obj )]( auto&& source ) {
                using arg_t = decltype( source );
                return second_closure( first_closure( std::forward<arg_t>( source ) ) );
            } };
    }
    else
    {
        // apply arg to given closure and return its result
        return std::forward<Closure>( closure_obj )( std::forward<Arg>( arg ) );
    }
}

UREACT_END_NAMESPACE

#endif // UREACT_CLOSURE_HPP
