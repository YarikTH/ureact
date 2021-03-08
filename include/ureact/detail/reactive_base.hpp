#pragma once

#include <memory>
#include <utility>

#include "ureact/detail/context.hpp"

namespace ureact
{
namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// reactive_base
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename node_t>
class reactive_base
{
public:
    // Default ctor
    reactive_base() = default;

    // Constructor
    explicit reactive_base( context* context )
        : m_context( context )
    {}

    // Copy ctor
    reactive_base( const reactive_base& ) = default;

    // Move ctor
    reactive_base( reactive_base&& other ) noexcept = default;

    // Explicit node ctor
    reactive_base( context* context, std::shared_ptr<node_t>&& ptr ) noexcept
        : m_context( context )
        , m_ptr( std::move( ptr ) )
    {}

    // Copy assignment
    reactive_base& operator=( const reactive_base& ) = default;

    // Move assignment
    reactive_base& operator=( reactive_base&& other ) noexcept = default;

    ~reactive_base() = default;

    bool is_valid() const
    {
        return m_ptr != nullptr;
    }

    bool equals( const reactive_base& other ) const
    {
        return this->m_ptr == other.m_ptr;
    }

    context* get_context() const
    {
        return m_context;
    }

protected:
    context* m_context = nullptr;
    std::shared_ptr<node_t> m_ptr;

    template <typename node_t_>
    friend const std::shared_ptr<node_t_>& get_node_ptr( const reactive_base<node_t_>& node );
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// get_node_ptr
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename node_t>
const std::shared_ptr<node_t>& get_node_ptr( const reactive_base<node_t>& node )
{
    return node.m_ptr;
}

} // namespace detail
} // namespace ureact
