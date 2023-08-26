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

#include <ureact/context.hpp>
#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Guard class to perform several changes atomically
 */
class UREACT_API UREACT_WARN_UNUSED_RESULT transaction
{
public:
    explicit transaction( context ctx );
    ~transaction();

    /*!
     * @brief Finish transaction before code scope is ended
     */
    void finish();

private:
    void finish_impl();

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
struct UREACT_API UREACT_WARN_UNUSED_RESULT default_transaction : public transaction
{
    default_transaction();
};

} // namespace default_context

UREACT_END_NAMESPACE

#if UREACT_HEADER_ONLY
#    include <ureact/detail/transaction.inl>
#endif

#endif // UREACT_TRANSACTION_HPP
