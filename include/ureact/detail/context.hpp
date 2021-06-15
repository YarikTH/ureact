#pragma once

#include "ureact/detail/context_internals.hpp"

namespace ureact
{

class context : protected detail::context_internals
{
public:
    template <typename F>
    void do_transaction( F&& func )
    {
        m_graph->do_transaction( std::forward<F>( func ) );
    }

    bool operator==( const context& rsh )
    {
        return this == &rsh;
    }

    bool operator!=( const context& rsh )
    {
        return this != &rsh;
    }

    friend detail::context_internals& _get_internals( context& ctx )
    {
        return ctx;
    }

    friend const detail::context_internals& _get_internals( const context& ctx )
    {
        return ctx;
    }
};

} // namespace ureact
