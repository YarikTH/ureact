//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_NODE_ID_VECTOR_HPP
#define UREACT_DETAIL_NODE_ID_VECTOR_HPP

#include <vector>

#include <ureact/detail/defines.hpp>
#include <ureact/detail/graph_interface.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

class node_id_vector
{
public:
    using value_type = node_id;
    using container_type = std::vector<value_type>;
    using iterator = container_type::iterator;

    void add( node_id id );
    void remove( node_id id );
    void clear();
    UREACT_WARN_UNUSED_RESULT bool empty() const;

    UREACT_WARN_UNUSED_RESULT iterator begin();
    UREACT_WARN_UNUSED_RESULT iterator end();

private:
    container_type m_data;
};

} // namespace detail

UREACT_END_NAMESPACE

#if UREACT_HEADER_ONLY
#    include <ureact/detail/node_id_vector.inl>
#endif

#endif //UREACT_DETAIL_NODE_ID_VECTOR_HPP
