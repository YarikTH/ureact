//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_UNIQUE_HPP
#define UREACT_UNIQUE_HPP

#include <ureact/detail/closure.hpp>
#include <ureact/filter.hpp>
#include <ureact/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct UniqueClosure : AdaptorClosure
{
/*!
 * @brief Filter out all except the first element from every consecutive group of equivalent elements
 *
 *  In other words: removes consecutive (adjacent) duplicates
 *
 *  Semantically equivalent of std::unique
 */
template <typename E>
UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<E>& source ) const
{
    return filter( source, [first = true, prev = E{}]( const E& e ) mutable {
        const bool pass = first || e != prev;
        first = false;
        prev = e;
        return pass;
    } );
}
};

} // namespace detail

inline constexpr detail::UniqueClosure unique;

UREACT_END_NAMESPACE

#endif // UREACT_UNIQUE_HPP
