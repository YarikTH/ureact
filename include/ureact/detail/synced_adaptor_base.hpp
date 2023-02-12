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
#include <ureact/signal_pack.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename Derived>
struct SyncedAdaptorBase : Adaptor
{
    template <typename E, typename Pred>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<E>& source, Pred&& pred ) const
    {
        return Derived{}( source, signal_pack<>(), std::forward<Pred>( pred ) );
    }

    template <typename... Deps, typename Pred>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const signal_pack<Deps...>& dep_pack, Pred&& pred ) const
    {
        return make_partial<Derived>( dep_pack, std::forward<Pred>( pred ) );
    }

    template <typename Pred>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Pred&& pred ) const
    {
        return make_partial<Derived>( std::forward<Pred>( pred ) );
    }
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_SYNCED_ADAPTOR_BASE_HPP
