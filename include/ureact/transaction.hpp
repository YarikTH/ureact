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

#include <ureact/detail/base.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Guard class to perform several changes atomically
 *
 */
class UREACT_WARN_UNUSED_RESULT transaction
{
public:
    explicit transaction( context& ctx )
        : self( _get_internals( ctx ).get_graph() )
    {
        ++self.m_transaction_level;
    }

    ~transaction()
    {
        --self.m_transaction_level;

        if( self.m_transaction_level == 0 )
        {
            self.propagate();
        }
    }

private:
    UREACT_MAKE_NONCOPYABLE( transaction );
    UREACT_MAKE_NONMOVABLE( transaction );

    detail::react_graph& self;
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

UREACT_END_NAMESPACE

#endif // UREACT_TRANSACTION_HPP
