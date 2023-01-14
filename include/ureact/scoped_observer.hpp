//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_SCOPED_OBSERVER_HPP
#define UREACT_SCOPED_OBSERVER_HPP

#include <ureact/ureact.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Takes ownership of an observer and automatically detaches it on scope exit
 */
class scoped_observer final
{
public:
    UREACT_MAKE_NONCOPYABLE( scoped_observer );
    UREACT_MAKE_MOVABLE( scoped_observer );

    /*!
     * @brief is not intended to be default constructive
     */
    scoped_observer() = delete;

    /*!
     * @brief Constructs instance from observer
     */
    scoped_observer( observer&& observer ) // NOLINT no explicit by design
        : m_observer( std::move( observer ) )
    {}

    /*!
     * @brief Destructor
     */
    ~scoped_observer()
    {
        m_observer.detach();
    }

    /*!
     * @brief Tests if this instance is linked to a node
     */
    UREACT_WARN_UNUSED_RESULT bool is_valid() const // TODO: check in tests
    {
        return m_observer.is_valid();
    }

private:
    observer m_observer;
};

UREACT_END_NAMESPACE

#endif //UREACT_SCOPED_OBSERVER_HPP
