//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_SYNCED_ADAPTOR_BASE_HPP
#define UREACT_DETAIL_SYNCED_ADAPTOR_BASE_HPP

#include <ureact/detail/adaptor.hpp>
#include <ureact/utility/signal_pack.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename Derived>
struct SyncedAdaptorBase : Adaptor
{
    template <typename E, typename F>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<E>& source, F&& func ) const
    {
        return Derived{}.operator()( source, signal_pack<>{}, std::forward<F>( func ) );
    }

    template <typename F, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const signal_pack<Deps...>& dep_pack, F&& func ) const
    {
        return make_partial<Derived>( dep_pack, std::forward<F>( func ) );
    }

    template <typename F>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( F&& func ) const
    {
        return make_partial<Derived>( std::forward<F>( func ) );
    }
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_SYNCED_ADAPTOR_BASE_HPP
