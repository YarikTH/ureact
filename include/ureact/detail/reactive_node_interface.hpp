#pragma once

#include <vector>
#include <limits>

namespace ureact
{
namespace detail
{

template <typename node_t>
class node_vector : public std::vector<node_t*>
{
public:
    void add( node_t& node )
    {
        node_vector::push_back( &node );
    }

    void remove( const node_t& node )
    {
        node_vector::erase( find( node ) );
    }

private:
    // To prevent including <algorithm> just to use std::find
    typename node_vector::const_iterator find( const node_t& node ) const
    {
        for ( auto it = node_vector::begin(), ite = node_vector::end(); it != ite; ++it )
        {
            if ( *it == &node )
            {
                return it;
            }
        }
        return node_vector::end();
    }
};

using turn_id_t = unsigned int;

class turn_t
{
public:
    turn_t() = default;

    turn_t( turn_id_t id )
        : m_id( id )
    {}

    turn_id_t id() const
    {
        return m_id;
    }

private:
    turn_id_t m_id = std::numeric_limits<turn_id_t>::max();
};

struct reactive_node_interface
{
    virtual ~reactive_node_interface() = default;

    virtual void tick( turn_t& turn ) = 0;
};

class reactive_node : public reactive_node_interface
{
public:
    int level{ 0 };
    int new_level{ 0 };
    bool queued{ false };

    node_vector<reactive_node> successors;
};

struct input_node_interface
{
    virtual ~input_node_interface() = default;

    virtual bool apply_input( turn_t& turn ) = 0;
};

} // namespace detail
} // namespace ureact
