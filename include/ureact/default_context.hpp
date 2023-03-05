//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DEFAULT_CONTEXT_HPP
#define UREACT_DEFAULT_CONTEXT_HPP

#include <ureact/context.hpp>
#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

namespace default_context
{

/**
 * @brief Return default context
 * Default contexts are thread_local
 */
inline context get()
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

#endif //UREACT_DEFAULT_CONTEXT_HPP
