#pragma once

#include "ureact/detail/turn_base.hpp"
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
    data_t    m_data;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// i_reactive_node
///////////////////////////////////////////////////////////////////////////////////////////////////
struct i_reactive_node
{
    virtual ~i_reactive_node() = default;
    
    virtual void    tick(turn_base& turn) = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// reactive_node
///////////////////////////////////////////////////////////////////////////////////////////////////
class reactive_node : public i_reactive_node
{
public:
    int     level       { 0 };
    int     new_level    { 0 };
    bool    queued      { false };

    node_vector<reactive_node>     successors;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// i_input_node
///////////////////////////////////////////////////////////////////////////////////////////////////
struct i_input_node
{
    virtual ~i_input_node() = default;

    virtual bool apply_input(turn_base& turn) = 0;
};

}}
