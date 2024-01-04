//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_NODE_ID_VECTOR_INL
#define UREACT_DETAIL_NODE_ID_VECTOR_INL

#include <cassert>

#include <ureact/detail/algorithm.hpp>
#include <ureact/detail/defines.hpp>
#include <ureact/detail/node_id_vector.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

UREACT_FUNC void node_id_vector::add( const node_id id )
{
    m_data.push_back( id );
}

UREACT_FUNC void node_id_vector::remove( const node_id id )
{
    const auto it = detail::find( m_data.begin(), m_data.end(), id );
    assert( it != m_data.end() );

    // Unstable erase algorithm
    // If we remove not the last element, then copy last element on erased position and remove last element
    const auto last_it = m_data.begin() + m_data.size() - 1;
    if( it != last_it )
    {
        *it = *last_it;
    }

    m_data.resize( m_data.size() - 1 );
}

UREACT_FUNC void node_id_vector::clear()
{
    m_data.clear();
}

UREACT_FUNC bool node_id_vector::empty() const
{
    return m_data.empty();
}

UREACT_FUNC node_id_vector::iterator node_id_vector::begin()
{
    return m_data.begin();
}

UREACT_FUNC node_id_vector::iterator node_id_vector::end()
{
    return m_data.end();
}

} // namespace detail

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_NODE_ID_VECTOR_INL
