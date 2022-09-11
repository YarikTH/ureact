//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_TAKE_DROP_HPP
#define UREACT_TAKE_DROP_HPP

#include "filter.hpp"

UREACT_BEGIN_NAMESPACE

namespace detail
{

/*!
 * @brief counter that counts down from N to 0
 *
 * prefix decrement operator decrements only to 0
 * example: for(countdown i{N}; i; --i )
 */
class countdown
{
public:
    explicit countdown( size_t value )
        : m_value( value )
    {}

    countdown& operator--()
    {
        m_value = dec( m_value );
        return *this;
    }

    const countdown operator--( int ) // NOLINT
    {
        countdown i{ m_value };
        m_value = dec( m_value );
        return i;
    }

    // checkable in boolean context
    explicit operator bool() const
    {
        return m_value;
    }

private:
    // decrement operator decrements only to 0
    [[nodiscard]] static size_t dec( const size_t value )
    {
        if( value == 0 ) // [[likely]]
            return 0;
        else
            return value - 1;
    }

    size_t m_value;
};

} // namespace detail

/*!
 * @brief Keeps first N elements from the source stream
 *
 *  Semantically equivalent of std::ranges::views::take
 */
template <typename E, typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT auto take( const events<E>& source, const N count )
{
    assert( count >= 0 );
    return filter( source,                                        //
        [i = detail::countdown( count )]( const auto& ) mutable { //
            return bool( i-- );
        } );
}

/*!
 * @brief Curried version of take(const events<E>& source, const size_t count)
 */
template <typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT auto take( const N count )
{
    assert( count >= 0 );
    return closure{ [count]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return take( std::forward<arg_t>( source ), count );
    } };
}

/*!
 * @brief Skips first N elements from the source stream
 *
 *  Semantically equivalent of std::ranges::views::drop
 */
template <typename E, typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT auto drop( const events<E>& source, const N count )
{
    assert( count >= 0 );
    return filter( source,                                        //
        [i = detail::countdown( count )]( const auto& ) mutable { //
            return !bool( i-- );
        } );
}

/*!
 * @brief Curried version of drop(const events<E>& source, const N count)
 */
template <typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT auto drop( const N count )
{
    assert( count >= 0 );
    return closure{ [count]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return drop( std::forward<arg_t>( source ), count );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_TAKE_DROP_HPP
