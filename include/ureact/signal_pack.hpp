//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_SIGNAL_PACK_HPP
#define UREACT_SIGNAL_PACK_HPP

#include <tuple>

#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

template <typename S>
class signal;

/*!
 * @brief A wrapper type for a tuple of signal references
 * @tparam Values types of signal values
 *
 *  Created with @ref with()
 */
template <typename... Values>
class signal_pack final
{
public:
    /*!
     * @brief Class to store signals instead of signal references
     */
    class stored
    {
    public:
        /*!
         * @brief Construct from signals
         */
        explicit stored( const signal<Values>&... deps )
            : data( std::tie( deps... ) )
        {}

        /*!
         * @brief The wrapped tuple
         */
        std::tuple<signal<Values>...> data;
    };

    /*!
     * @brief Construct from signals
     */
    explicit signal_pack( const signal<Values>&... deps )
        : data( std::tie( deps... ) )
    {}

    /*!
     * @brief Construct from stored signals
     */
    explicit signal_pack( const stored& value )
        : data( std::apply(
            []( const signal<Values>&... deps ) { return std::tie( deps... ); }, value.data ) )
    {}

    /*!
     * @brief Convert signal references to signals so they can be stored
     */
    UREACT_WARN_UNUSED_RESULT stored store() const
    {
        return std::apply(
            []( const signal<Values>&... deps ) { return stored{ deps... }; }, data );
    }

    /*!
     * @brief The wrapped tuple
     */
    std::tuple<const signal<Values>&...> data;
};

/*!
 * @brief Utility function to create a signal_pack from given signals
 * @tparam Values types of signal values
 *
 *  Creates a signal_pack from the signals passed as deps.
 *  Semantically, this is equivalent to std::tie.
 */
template <typename... Values>
UREACT_WARN_UNUSED_RESULT auto with( const signal<Values>&... deps )
{
    return signal_pack<Values...>( deps... );
}

UREACT_END_NAMESPACE

#endif //UREACT_SIGNAL_PACK_HPP
