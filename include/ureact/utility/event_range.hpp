//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_UTILITY_EVENT_RANGE_HPP
#define UREACT_UTILITY_EVENT_RANGE_HPP

#include <vector>

#include <ureact/detail/defines.hpp>
#include <ureact/utility/unit.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Represents a range of events. It is serves as an adaptor to the underlying event container of a source node
 *
 *  An instance of event_range holds a reference to the wrapped container and selectively exposes functionality
 *  that allows to iterate over its events without modifying it.
 *
 *  TODO: think about making values movable, so values would be processed more efficiently
 */
template <typename E = unit>
class event_range final
{
public:
    using const_iterator = typename std::vector<E>::const_iterator;
    using const_reverse_iterator = typename std::vector<E>::const_reverse_iterator;
    using size_type = typename std::vector<E>::size_type;

    /*!
     * @brief Constructor
     */
    explicit event_range( const std::vector<E>& data )
        : m_data( data )
    {}

    /*!
     * @brief Returns a random access const iterator to the beginning
     */
    UREACT_WARN_UNUSED_RESULT const_iterator begin() const
    {
        return m_data.begin();
    }

    /*!
     * @brief Returns a random access const iterator to the end
     */
    UREACT_WARN_UNUSED_RESULT const_iterator end() const
    {
        return m_data.end();
    }

    /*!
     * @brief Returns a reverse random access const iterator to the beginning
     */
    UREACT_WARN_UNUSED_RESULT const_reverse_iterator rbegin() const
    {
        return m_data.rbegin();
    }

    /*!
     * @brief Returns a reverse random access const iterator to the end
     */
    UREACT_WARN_UNUSED_RESULT const_reverse_iterator rend() const
    {
        return m_data.rend();
    }

    /*!
     * @brief Returns the number of events
     */
    UREACT_WARN_UNUSED_RESULT size_type size() const
    {
        return m_data.size();
    }

    /*!
     * @brief Checks whether the container is empty
     */
    UREACT_WARN_UNUSED_RESULT bool empty() const
    {
        return m_data.empty();
    }

private:
    const std::vector<E>& m_data;
};

UREACT_END_NAMESPACE

#endif //UREACT_UTILITY_EVENT_RANGE_HPP
