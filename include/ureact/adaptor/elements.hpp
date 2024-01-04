//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_ELEMENTS_HPP
#define UREACT_ADAPTOR_ELEMENTS_HPP

#include <tuple>

#include <ureact/adaptor/transform.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Takes event stream of tuple-like values and creates a new event stream with a value-type of the N-th element of received value-type
 * 
 *  For every event e in source, emit t = std::get<N>(e).
 */
template <size_t N>
inline constexpr auto elements = transform( []( const auto& e ) { return std::get<N>( e ); } );

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_ELEMENTS_HPP
