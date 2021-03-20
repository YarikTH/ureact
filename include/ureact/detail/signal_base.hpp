#pragma once

#include <utility>

#include "ureact/detail/reactive_base.hpp"
#include "ureact/detail/reactive_input.hpp"
#include "ureact/detail/graph/signal_node.hpp"
#include "ureact/detail/graph/var_node.hpp"

namespace ureact
{
namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_base
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class signal_base : public reactive_base<signal_node<S>>
{
public:
    // Default ctor
    signal_base() = default;

    template <typename T>
    explicit signal_base( T&& t )
        : signal_base::reactive_base( std::forward<T>( t ) )
    {}

protected:
    const S& get_value() const
    {
        return this->m_ptr->value_ref();
    }
};

} // namespace detail
} // namespace ureact
