//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_EVENT_EMITTER_HPP
#define UREACT_EVENT_EMITTER_HPP

#include <vector>

#include <ureact/detail/defines.hpp>
#include <ureact/unit.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Represents output stream of events.
 *
 *  It is std::back_insert_iterator analog.
 *  Additionally to std::back_insert_iterator interface it provides operator<< overload
 */
template <typename E = unit>
class event_emitter final
{
public:
    using container_type = std::vector<E>;
    using iterator_category = std::output_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = E;
    using pointer = value_type*;
    using reference = value_type&;

    /*!
     * @brief Constructor
     */
    explicit event_emitter( container_type& container )
        : m_container( &container )
    {}

    // clang-format off
    event_emitter& operator*()       { return *this; }
    event_emitter& operator++()      { return *this; }
    event_emitter  operator++( int ) { return *this; } // NOLINT
    // clang-format on

    /*!
     * @brief Adds e to the queue of outgoing events
     */
    event_emitter& operator=( const E& e )
    {
        m_container->push_back( e );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     *
     * Specialization of operator=(const E& e) for rvalue
     */
    event_emitter& operator=( E&& e )
    {
        m_container->push_back( std::move( e ) );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     */
    event_emitter& operator<<( const E& e )
    {
        m_container->push_back( e );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     *
     * Specialization of operator<<(const E& e) for rvalue
     */
    event_emitter& operator<<( E&& e )
    {
        m_container->push_back( std::move( e ) );
        return *this;
    }

private:
    container_type* m_container;
};

UREACT_END_NAMESPACE

#endif //UREACT_EVENT_EMITTER_HPP
