//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_CONTEXT_INL
#define UREACT_DETAIL_CONTEXT_INL

#include <memory>

#include <ureact/detail/defines.hpp>
#include <ureact/detail/graph_impl.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

UREACT_FUNC context_internals::context_internals( std::shared_ptr<react_graph> graph )
    : m_graph_ptr( std::move( graph ) )
{}

UREACT_FUNC react_graph& context_internals::get_graph()
{
    return *m_graph_ptr;
}

UREACT_FUNC const react_graph& context_internals::get_graph() const
{
    return *m_graph_ptr;
}

} // namespace detail

UREACT_FUNC context::context( std::shared_ptr<detail::react_graph> graph )
    : detail::context_internals( std::move( graph ) )
{}

namespace default_context
{

UREACT_FUNC context get()
{
    thread_local static std::weak_ptr<detail::react_graph> s_instance;

    auto graphPtr = s_instance.lock();

    if( !graphPtr )
    {
        s_instance = graphPtr = std::make_shared<detail::react_graph>();
    }

    return context{ std::move( graphPtr ) };
}

} // namespace default_context

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_CONTEXT_INL
