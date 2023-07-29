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
        , m_self( get_internals( m_context ).get_graph() )
    {
        ++m_self.m_transaction_level;
    }

    ~transaction()
    {
        --m_self.m_transaction_level;

        if( m_self.m_transaction_level == 0 )
        {
            m_self.propagate();
        }
    }

private:
    UREACT_MAKE_NONCOPYABLE( transaction );
    UREACT_MAKE_NONMOVABLE( transaction );

    context m_context;
    detail::react_graph& m_self;
};

/*!
 * @brief Perform several changes atomically
 * @tparam F type of passed functor
 * @tparam Args types of additional arguments passed to functor F
 *
 *  Can pass additional arguments to the functiona and optionally return a result
 */
template <typename F,
    typename... Args,
    class = std::enable_if_t<std::is_invocable_v<F&&, Args&&...>>>
UREACT_WARN_UNUSED_RESULT auto do_transaction( context& ctx, F&& func, Args&&... args )
{
    transaction _{ ctx };

    if constexpr( std::is_same_v<std::invoke_result_t<F&&, Args&&...>, void> )
    {
        std::invoke( std::forward<F>( func ), std::forward<Args>( args )... );
    }
    else
    {
        return std::invoke( std::forward<F>( func ), std::forward<Args>( args )... );
    }
}

namespace default_context
{

/*!
 * @brief Guard class to perform several changes atomically
 *
 */
struct UREACT_WARN_UNUSED_RESULT default_transaction : ureact::transaction
{
    default_transaction()
        : ureact::transaction( default_context::get() )
    {}
};

/*!
 * @brief Perform several changes atomically
 * @tparam F type of passed functor
 * @tparam Args types of additional arguments passed to functor F
 *
 *  Can pass additional arguments to the functiona and optionally return a result
 */
template <typename F,
    typename... Args,
    class = std::enable_if_t<std::is_invocable_v<F&&, Args&&...>>>
UREACT_WARN_UNUSED_RESULT auto do_transaction( F&& func, Args&&... args )
{
    default_transaction _;

    if constexpr( std::is_same_v<std::invoke_result_t<F&&, Args&&...>, void> )
    {
        std::invoke( std::forward<F>( func ), std::forward<Args>( args )... );
    }
    else
    {
        return std::invoke( std::forward<F>( func ), std::forward<Args>( args )... );
    }
}

} // namespace default_context

UREACT_END_NAMESPACE

#endif // UREACT_TRANSACTION_HPP
