//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_OBSERVER_HPP
#define UREACT_OBSERVER_HPP

#include <utility>

#include <ureact/detail/node_base.hpp>
#include <ureact/detail/observer_node.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

class observer_internals
{
public:
    observer_internals() = default;

    explicit observer_internals( std::shared_ptr<observer_node> node )
        : m_node( std::move( node ) )
    {}

    UREACT_MAKE_COPYABLE( observer_internals );
    UREACT_MAKE_MOVABLE( observer_internals );

    UREACT_WARN_UNUSED_RESULT std::shared_ptr<observer_node>& get_node_ptr()
    {
        return m_node;
    }

    UREACT_WARN_UNUSED_RESULT const std::shared_ptr<observer_node>& get_node_ptr() const
    {
        return m_node;
    }

    UREACT_WARN_UNUSED_RESULT node_id get_node_id() const
    {
        return m_node->get_node_id();
    }

protected:
    UREACT_WARN_UNUSED_RESULT react_graph& get_graph() const
    {
        assert( m_node != nullptr && "Should be attached to a node" );
        return get_internals( m_node->get_context() ).get_graph();
    }

    /// Pointer to owned node
    std::shared_ptr<observer_node> m_node;
};

} // namespace detail

/*!
 * @brief Shared pointer like object that holds a strong reference to the observer node
 *
 *  An instance of this class provides a unique handle to an observer which can
 *  be used to detach it explicitly
 */
class observer final : protected detail::observer_internals
{
public:
    UREACT_MAKE_COPYABLE( observer );
    UREACT_MAKE_MOVABLE( observer );

    /*!
     * @brief Default constructor
     */
    observer() = default;

    /*!
     * @brief Manually detaches the linked observer node from its subject
     */
    void detach()
    {
        assert( is_valid() );
        get_node_ptr()->detach_observer();
    }

    /*!
     * @brief Tests if this instance is linked to a node
     */
    UREACT_WARN_UNUSED_RESULT bool is_valid() const
    {
        return this->get_node_ptr() != nullptr;
    }

    /*!
     * @brief Return internals. Not intended to use in user code
     */
    UREACT_WARN_UNUSED_RESULT friend detail::observer_internals& get_internals( observer& obs )
    {
        return obs;
    }

    /*!
     * @brief Return internals. Not intended to use in user code
     */
    UREACT_WARN_UNUSED_RESULT friend const detail::observer_internals& get_internals(
        const observer& obs )
    {
        return obs;
    }

protected:
    /*!
     * @brief Construct from the given node
     */
    explicit observer( std::shared_ptr<detail::observer_node> node )
        : observer_internals( std::move( node ) )
    {}

    template <typename Ret, typename Node, typename... Args>
    friend Ret detail::create_wrapped_node( Args&&... args );
};

UREACT_END_NAMESPACE

#endif //UREACT_OBSERVER_HPP
