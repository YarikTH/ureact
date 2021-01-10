#pragma once

#include "ureact/detail/graph/observable_node.hpp"

namespace ureact { namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename S
>
class signal_node : public observable_node
{
public:
    explicit signal_node(context* context) :
        observable_node( context )
    {
    }

    template <typename T>
    signal_node(context* context, T&& value) :
        observable_node( context ),
        m_value( std::forward<T>( value ) )
    {}

    const S& value_ref() const
    {
        return m_value;
    }

protected:
    S m_value;
};

template <typename S>
using signal_node_ptr_t = std::shared_ptr<signal_node<S>>;

}}
