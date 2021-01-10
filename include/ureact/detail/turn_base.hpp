#pragma once

namespace ureact { namespace detail {

using turn_id_t = unsigned int;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// turn_base
///////////////////////////////////////////////////////////////////////////////////////////////////
class turn_base
{
public:
    turn_base() = default;
    
    turn_base(turn_id_t id) :
        m_id( id )
    {}

    turn_id_t id() const { return m_id; }

private:
    turn_id_t    m_id = -1u;
};

}}
