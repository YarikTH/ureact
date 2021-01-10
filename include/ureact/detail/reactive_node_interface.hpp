#pragma once

#include "ureact/detail/turn_t.hpp"
#include <limits>
#include <algorithm>

namespace ureact { namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// node_vector
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename node_t>
class node_vector
{
private:
    using data_t = std::vector<node_t *>;

public:
    void add(node_t& node)
    {
        m_data.push_back( &node );
    }

    void remove(const node_t& node)
    {
        m_data.erase( std::find( m_data.begin(), m_data.end(), &node ) );
    }

    using iterator = typename data_t::iterator;
    using const_iterator = typename data_t::const_iterator;

    iterator    begin() { return m_data.begin(); }
    iterator    end()   { return m_data.end(); }

    const_iterator begin() const    { return m_data.begin(); }
    const_iterator end() const      { return m_data.end(); }

private:
    data_t    m_data {};
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// reactive_node_interface
///////////////////////////////////////////////////////////////////////////////////////////////////
struct reactive_node_interface
{
    virtual ~reactive_node_interface() = default;
    
    virtual void    tick( turn_t& turn ) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// reactive_node
///////////////////////////////////////////////////////////////////////////////////////////////////
class reactive_node : public reactive_node_interface
{
public:
    int     level       { 0 };
    int     new_level    { 0 };
    bool    queued      { false };

    node_vector<reactive_node>     successors;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// input_node_interface
///////////////////////////////////////////////////////////////////////////////////////////////////
struct input_node_interface
{
    virtual ~input_node_interface() = default;

    virtual bool apply_input( turn_t& turn ) = 0;
};

}}
