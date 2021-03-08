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

    template <typename T>
    void set_value( T&& new_value ) const
    {
        auto node = static_cast<var_node<S>*>(this->m_ptr.get());
        auto& input_manager = node->get_context()->get_input_manager();
        input_manager.add_input( *node, std::forward<T>(new_value) );
    }

    template <typename F>
    void modify_value( const F& func ) const
    {
        auto node = static_cast<var_node<S>*>(this->m_ptr.get());
        auto& input_manager = node->get_context()->get_input_manager();
        input_manager.modify_input( *node, func );
    }
};

} // namespace detail
} // namespace ureact
