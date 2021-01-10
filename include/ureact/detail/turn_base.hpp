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
        id_( id )
    {}

    turn_id_t id() const { return id_; }

private:
    turn_id_t    id_ = -1u;
};

}}
