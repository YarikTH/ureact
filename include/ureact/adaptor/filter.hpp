//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_FILTER_HPP
#define UREACT_ADAPTOR_FILTER_HPP

#include <ureact/adaptor/detail/synced_adaptor_base.hpp>
#include <ureact/adaptor/process.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct FilterAdaptor : SyncedAdaptorBase<FilterAdaptor>
{
    /*!
	 * @brief Create a new event stream that filters events from other stream
	 *
	 *  For every event e in source, emit e if pred(e, deps...) == true.
	 *  Synchronized values of signals in dep_pack are passed to op as additional arguments.
	 *
	 *  The signature of pred should be equivalent to:
	 *  * bool pred(const E&, const Deps& ...)
	 *
	 *  Semantically equivalent of ranges::filter
	 *
	 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
	 */
    template <typename E, typename Pred, typename... Deps>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred ) const
    {
        return process<E>( source,
            dep_pack, //
            [pred = std::forward<Pred>( pred )](
                event_range<E> range, const Deps... deps, event_emitter<E> out ) mutable {
                for( const auto& e : range )
                    if( std::invoke( pred, e, deps... ) )
                        out << e;
            } );
    }

    using SyncedAdaptorBase::operator();
};

} // namespace detail

inline constexpr detail::FilterAdaptor filter;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_FILTER_HPP
