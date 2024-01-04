//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_UTILITY_SIGNAL_PACK_HPP
#define UREACT_UTILITY_SIGNAL_PACK_HPP

#include <tuple>

#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

template <typename S>
class signal;

/*!
 * @brief A wrapper type for a tuple of signals
 * @tparam Values types of signal values
 *
 *  Created with @ref with()
 */
template <typename... Values>
class signal_pack final
{
public:
    /*!
     * @brief Construct from signals
     */
    explicit signal_pack( const signal<Values>&... deps )
        : data( std::tie( deps... ) )
    {}

    /*!
     * @brief The wrapped tuple
     */
    std::tuple<signal<Values>...> data;
};

/*!
 * @brief Utility function to create a signal_pack from given signals
 * @tparam Values types of signal values
 *
 *  Creates a signal_pack from the signals passed as deps.
 *  Semantically, this is equivalent to std::make_tuple.
 *  
 *  TODO: receive universal references and make valid signal_pack from them
 */
template <typename... Values>
UREACT_WARN_UNUSED_RESULT auto with( const signal<Values>&... deps )
{
    return signal_pack<Values...>( deps... );
}

UREACT_END_NAMESPACE

#endif //UREACT_UTILITY_SIGNAL_PACK_HPP
