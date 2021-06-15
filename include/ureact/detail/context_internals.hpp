#pragma once

#include <memory>

#include "ureact/detail/react_graph.hpp"

namespace ureact
{
namespace detail
{

class context_internals
{
public:
    context_internals()
        : m_graph( new react_graph() )
    {}

    react_graph& get_graph()
    {
        return *m_graph;
    }

    const react_graph& get_graph() const
    {
        return *m_graph;
    }

private:
    std::unique_ptr<react_graph> m_graph;
};

} // namespace detail
} // namespace ureact
