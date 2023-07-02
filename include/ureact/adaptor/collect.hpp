//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_COLLECT_HPP
#define UREACT_ADAPTOR_COLLECT_HPP

#include <ureact/adaptor/fold.hpp>
#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/container_type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <template <typename...> class ContT>
struct CollectClosure : AdaptorClosure
{
    /*!
	 * @brief Collects received events into signal<ContT<E>>
	 *
	 *  Type of resulting container must be specified explicitly, i.e. collect<std::vector>(src).
	 *  Container type ContT should has either push_back(const E&) method or has insert(const E&) method.
	 *  Mostly intended for testing purpose.
	 *
	 *  Semantically equivalent of ranges::to
	 *
	 *  @warning Use with caution, because there is no way to finalize its value, or to ensure it destroyed
	 *           because any observer or signal/events node will prolong its lifetime.
	 */
    template <class E, class Cont = ContT<E>>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<E>& source ) const
        -> signal<Cont>
    {
        return fold( source,
            Cont{},                         //
            []( const E& e, Cont& accum ) { //
                if constexpr( has_push_back_method_v<Cont, E> )
                    accum.push_back( e );
                else if constexpr( has_insert_method_v<Cont, E> )
                    accum.insert( e );
                else
                    static_assert( always_false<Cont, E>, "Unsupported container" );
            } );
    }
};

} // namespace detail

template <template <typename...> class ContT>
inline constexpr detail::CollectClosure<ContT> collect;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_COLLECT_HPP
