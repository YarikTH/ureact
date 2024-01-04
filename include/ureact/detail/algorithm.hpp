//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_ALGORITHM_HPP
#define UREACT_DETAIL_ALGORITHM_HPP

#include <ureact/detail/defines.hpp>

#ifdef UREACT_USE_STD_ALGORITHM
#    include <algorithm>
#else
#    include <cstddef>
#    include <utility>
#endif

UREACT_BEGIN_NAMESPACE

// Partial alternative to <algorithm> is provided and used by default because library requires
// only a few algorithms while standard <algorithm> is quite bloated
// also `next` and `distance` from <iterator> are added, because standard <iterator> is even more bloated
namespace detail
{

#if defined( UREACT_USE_STD_ALGORITHM )

using std::find;
using std::partition;
using std::sort;

#else

// Code based on possible implementation at
// https://en.cppreference.com/w/cpp/algorithm/find
template <typename ForwardIt, typename Value>
ForwardIt find( ForwardIt first, ForwardIt last, const Value& val )
{
    for( auto it = first, ite = last; it != ite; ++it )
    {
        if( *it == val )
        {
            return it;
        }
    }
    return last;
}

// Code based on possible implementation at
// https://en.cppreference.com/w/cpp/algorithm/find
template <typename ForwardIt, typename Pred>
ForwardIt find_if_not( ForwardIt first, ForwardIt last, Pred pred )
{
    for( ; first != last; ++first )
    {
        if( !pred( *first ) )
        {
            return first;
        }
    }
    return last;
}

// Code based on possible implementation at
// https://en.cppreference.com/w/cpp/algorithm/iter_swap
template <typename LhsForwardIt, typename RhsForwardIt>
void iter_swap( LhsForwardIt a, RhsForwardIt b )
{
    using std::swap;
    swap( *a, *b );
}

// random_access_iterator_tag version of std::next()
template <typename LhsForwardIt>
auto next( LhsForwardIt iter, size_t n = 1 )
{
    iter += n;
    return iter;
}

// random_access_iterator_tag version of std::distance()
template <typename InputIter>
auto distance( InputIter first, InputIter last )
{
    return last - first;
}

// Code based on possible implementation at
// https://en.cppreference.com/w/cpp/algorithm/partition
template <typename ForwardIt, typename Pred>
ForwardIt partition( ForwardIt first, ForwardIt last, Pred pred )
{
    first = detail::find_if_not( first, last, pred );
    if( first == last )
    {
        return first;
    }

    for( ForwardIt i = detail::next( first ); i != last; ++i )
    {
        if( pred( *i ) )
        {
            detail::iter_swap( i, first );
            ++first;
        }
    }
    return first;
}

// Code based on quicksort example from
// https://en.cppreference.com/w/cpp/algorithm/partition
template <class ForwardIt>
void sort( ForwardIt first, ForwardIt last )
{
    if( first == last )
        return;
    const auto pivot = *detail::next( first, detail::distance( first, last ) / 2 );
    const auto middle1
        = detail::partition( first, last, [pivot]( const auto& em ) { return em < pivot; } );
    const auto middle2
        = detail::partition( middle1, last, [pivot]( const auto& em ) { return !( pivot < em ); } );
    detail::sort( first, middle1 );
    detail::sort( middle2, last );
}

#endif

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_ALGORITHM_HPP
