//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_TRANSACTION_INL
#define UREACT_DETAIL_TRANSACTION_INL

#include <cassert>

#include <ureact/context.hpp>
#include <ureact/detail/defines.hpp>
#include <ureact/detail/graph_impl.hpp>
#include <ureact/transaction.hpp>

UREACT_BEGIN_NAMESPACE

UREACT_FUNC transaction::transaction( context ctx )
    : m_context( std::move( ctx ) )
{
    auto& graph = get_internals( m_context ).get_graph();
    assert( !graph.is_propagation_in_progress()
            && "Can't start transaction in the middle of the change propagation process" );
    graph.start_transaction();
}

UREACT_FUNC transaction::~transaction()
{
    finish_impl();
}

UREACT_FUNC void transaction::finish()
{
    finish_impl();
}

UREACT_FUNC void transaction::finish_impl()
{
    if( !m_finished )
    {
        get_internals( m_context ).get_graph().finish_transaction();
        m_finished = true;
    }
}

namespace default_context
{

UREACT_FUNC default_transaction::default_transaction()
    : transaction( default_context::get() )
{}

} // namespace default_context

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_TRANSACTION_INL
