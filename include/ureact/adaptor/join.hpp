//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_JOIN_HPP
#define UREACT_ADAPTOR_JOIN_HPP

#include <ureact/adaptor/process.hpp>
#include <ureact/detail/container_type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct JoinClosure : adaptor_closure
{
    /*!
	 * @brief Emits the sequence obtained from flattening received event value
	 */
    template <typename InE>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<InE>& source ) const
    {
        using E = container_value_t<InE>;
        return process<E>( source, //
            []( event_range<InE> range, event_emitter<E> out ) {
                for( const auto& e : range )
                    for( const auto& i : e )
                        out << i;
            } );
    }
};

} // namespace detail

inline constexpr detail::JoinClosure join;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_JOIN_HPP
