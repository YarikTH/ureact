//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_TAKE_DROP_HPP
#define UREACT_TAKE_DROP_HPP

#include <ureact/detail/closure.hpp>
#include <ureact/filter.hpp>
#include <ureact/type_traits.hpp>

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
    UREACT_WARN_UNUSED_RESULT static size_t dec( const size_t value )
    {
        if( value == 0 ) // [[likely]]
            return 0;
        else
            return value - 1;
    }

    size_t m_value;
};

template <typename Derived>
struct TakeDropAdaptorBase : Adaptor
{
    template <typename N, class = std::enable_if_t<std::is_integral_v<N>>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const N count ) const
    {
        return make_partial<Derived>( count );
    }
};

struct TakeAdaptor : TakeDropAdaptorBase<TakeAdaptor>
{

/*!
 * @brief Keeps first N elements from the source stream
 *
 *  Semantically equivalent of std::ranges::views::take
 */
template <typename E, typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<E>& source, const N count ) const
{
    assert( count >= 0 );
    return filter( source,                                        //
        [i = detail::countdown( count )]( const auto& ) mutable { //
            return bool( i-- );
        } );
}

    using TakeDropAdaptorBase::operator();
};

struct DropAdaptor : TakeDropAdaptorBase<DropAdaptor>
{

/*!
 * @brief Skips first N elements from the source stream
 *
 *  Semantically equivalent of std::ranges::views::drop
 */
template <typename E, typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<E>& source, const N count ) const
{
    assert( count >= 0 );
    return filter( source,                                        //
        [i = detail::countdown( count )]( const auto& ) mutable { //
            return !bool( i-- );
        } );
}

    using TakeDropAdaptorBase::operator();
};

} // namespace detail

/*!
 * @brief Keeps first N elements from the source stream
 *
 *  Semantically equivalent of std::ranges::views::take
 */
inline constexpr detail::TakeAdaptor take;

/*!
 * @brief Skips first N elements from the source stream
 *
 *  Semantically equivalent of std::ranges::views::drop
 */
inline constexpr detail::DropAdaptor drop;

UREACT_END_NAMESPACE

#endif // UREACT_TAKE_DROP_HPP
