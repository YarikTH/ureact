//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_TRANSFORM_HPP
#define UREACT_ADAPTOR_TRANSFORM_HPP

#include <ureact/adaptor/process.hpp>
#include <ureact/detail/synced_adaptor_base.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct TransformAdaptor : SyncedAdaptorBase<TransformAdaptor>
{
    /*!
	 * @brief Create a new event stream that transforms events from other stream
	 *
	 *  For every event e in source, emit t = func(e, deps...).
	 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
	 *
	 *  The signature of func should be equivalent to:
	 *  * T func(const E&, const Deps& ...)
	 *
	 *  Semantically equivalent of ranges::transform
	 *
	 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
	 */
    template <typename InE,
        typename F,
        typename... Deps,
        typename OutE = std::invoke_result_t<F, InE, Deps...>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<InE>& source, const signal_pack<Deps...>& dep_pack, F&& func ) const
    {
        return process<OutE>( source,
            dep_pack, //
            [func = std::forward<F>( func )](
                event_range<InE> range, event_emitter<OutE> out, const auto... deps ) mutable {
                for( const auto& e : range )
                    out << std::invoke( func, e, deps... );
            } );
    }

    using SyncedAdaptorBase::operator();
};

} // namespace detail

inline constexpr detail::TransformAdaptor transform;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_TRANSFORM_HPP
