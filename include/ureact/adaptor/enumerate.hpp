//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_ENUMERATE_HPP
#define UREACT_ADAPTOR_ENUMERATE_HPP

#include <tuple>

#include <ureact/adaptor/transform.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct EnumerateAdaptorClosure : core::adaptor_closure
{
    template <typename E>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const events<E>& source ) const
    {
        return transform( source,                            //
            [index = size_t{ 0 }]( const auto& e ) mutable { //
                return std::make_tuple( index++, e );
            } );
    }
};

} // namespace detail

/*!
 * @brief Takes event stream and creates a new event stream with a tuple of both the element's position and its value
 * 
 *  For every event e in source, emit std::tuple<size_t, E>(index++, e).
 */
inline constexpr detail::EnumerateAdaptorClosure enumerate;

UREACT_END_NAMESPACE

#endif //UREACT_ADAPTOR_ENUMERATE_HPP
