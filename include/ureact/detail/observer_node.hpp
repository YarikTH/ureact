//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_OBSERVER_NODE_HPP
#define UREACT_DETAIL_OBSERVER_NODE_HPP

#include <ureact/detail/graph_interface.hpp>
#include <ureact/detail/node.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

class observer_node
    : public node
    , public observer_interface
{
public:
    explicit observer_node( const context& context )
        : node( context )
    {}
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_OBSERVER_NODE_HPP
