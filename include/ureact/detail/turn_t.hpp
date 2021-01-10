#pragma once

namespace ureact
{
namespace detail
{

using turn_id_t = unsigned int;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// turn_t
///////////////////////////////////////////////////////////////////////////////////////////////////
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
    turn_id_t m_id = -1u;
};

} // namespace detail
} // namespace ureact
