#pragma once

#include <cassert>

#include "ureact/detail/graph/signal_node.hpp"

namespace ureact
{
namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// var_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class var_node
    : public signal_node<S>
    , public input_node_interface
{
public:
    template <typename T>
    explicit var_node( context* context, T&& value )
        : var_node::signal_node( context, std::forward<T>( value ) )
        , m_new_value( value )
    {
        var_node::signal_node::get_context()->on_node_create( *this );
    }

    var_node( const var_node& ) = delete;
    var_node& operator=( const var_node& ) = delete;
    var_node( var_node&& ) noexcept = delete;
    var_node& operator=( var_node&& ) noexcept = delete;

    ~var_node() override
    {
        var_node::signal_node::get_context()->on_node_destroy( *this );
    }

    // LCOV_EXCL_START
    void tick( turn_t& /*turn*/ ) override
    {
        assert( false && "Ticked var_node" );
    }
    // LCOV_EXCL_STOP

    template <typename V>
    void request_add_input( V&& new_value )
    {
        auto& input_manager = var_node::signal_node::get_context()->get_input_manager();
        input_manager.add_input( *this, std::forward<V>( new_value ) );
    }

    template <typename F>
    void request_modify_input( F& func )
    {
        auto& input_manager = var_node::signal_node::get_context()->get_input_manager();
        input_manager.modify_input( *this, std::forward<F>( func ) );
    }

    template <typename V>
    void add_input( V&& new_value )
    {
        m_new_value = std::forward<V>( new_value );

        m_is_input_added = true;

        // m_is_input_added takes precedences over m_is_input_modified
        // the only difference between the two is that m_is_input_modified doesn't/can't compare
        m_is_input_modified = false;
    }

    // This is signal-specific
    template <typename F>
    void modify_input( F& func )
    {
        // There hasn't been any set(...) input yet, modify.
        if ( !m_is_input_added )
        {
            func( this->m_value );

            m_is_input_modified = true;
        }
        // There's a new_value, modify new_value instead.
        // The modified new_value will handled like before, i.e. it'll be compared to m_value
        // in apply_input
        else
        {
            func( m_new_value );
        }
    }

    bool apply_input( turn_t& turn ) override
    {
        if ( m_is_input_added )
        {
            m_is_input_added = false;

            if ( !equals( this->m_value, m_new_value ) )
            {
                this->m_value = std::move( m_new_value );
                var_node::get_context()->on_input_change( *this, turn );
                return true;
            }
            return false;
        }
        if ( m_is_input_modified )
        {
            m_is_input_modified = false;

            var_node::get_context()->on_input_change( *this, turn );
            return true;
        }
        return false;
    }

private:
    S m_new_value;
    bool m_is_input_added = false;
    bool m_is_input_modified = false;
};

} // namespace detail
} // namespace ureact
