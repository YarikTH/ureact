#pragma once

#include "ureact/detail/graph/node_base.hpp"
#include "ureact/detail/observer_base.hpp"

namespace ureact { namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// observer_node
///////////////////////////////////////////////////////////////////////////////////////////////////
class observer_node :
    public node_base,
    public i_observer
{
public:
    explicit observer_node(context* context)
        : node_base( context )
    {
    }
};

}}
