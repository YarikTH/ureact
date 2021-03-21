#pragma once

#include "ureact/detail/graph/node_base.hpp"
#include "ureact/detail/observer_base.hpp"

namespace ureact
{
namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// observable_node
///////////////////////////////////////////////////////////////////////////////////////////////////
class observable_node
    : public node_base
    , public observable
{
public:
    explicit observable_node( context& context )
        : node_base( context )
    {}
};

} // namespace detail
} // namespace ureact
