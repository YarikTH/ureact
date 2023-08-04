//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_TRANSACTION_HPP
#define UREACT_TRANSACTION_HPP

#include <functional>
#include <type_traits>

#include <ureact/context.hpp>
#include <ureact/detail/defines.hpp>
#include <ureact/detail/graph_impl.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Guard class to perform several changes atomically
 *
 */
class UREACT_WARN_UNUSED_RESULT transaction
{
public:
    explicit transaction( context ctx )
        : m_context( std::move( ctx ) )
    {
        get_internals( m_context ).get_graph().start_transaction();
    }

    ~transaction()
    {
        finish_impl();
    }

    /*!
     * @brief Finish transaction before code scope is ended
     */
    void finish()
    {
        finish_impl();
    }

private:
    void finish_impl()
    {
        if( !m_finished )
        {
            get_internals( m_context ).get_graph().finish_transaction();
            m_finished = true;
        }
    }

    UREACT_MAKE_NONCOPYABLE( transaction );
    UREACT_MAKE_NONMOVABLE( transaction );

    context m_context;
    bool m_finished = false;
};

namespace default_context
{

/*!
 * @brief Guard class to perform several changes atomically
 * 
 * Named differently from "transaction", so it is possible to use them both,
 * where both ureact and ureact::default_context namespaces are used
 */
struct UREACT_WARN_UNUSED_RESULT default_transaction : ureact::transaction
{
    default_transaction()
        : ureact::transaction( default_context::get() )
    {}
};

} // namespace default_context

UREACT_END_NAMESPACE

#endif // UREACT_TRANSACTION_HPP
