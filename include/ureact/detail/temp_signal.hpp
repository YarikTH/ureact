//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_TEMP_SIGNAL_HPP
#define UREACT_DETAIL_TEMP_SIGNAL_HPP

#include <cassert>

#include <ureact/signal.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S, typename Op>
class signal_op_node final : public signal_node<S>
{
public:
    template <typename... Args>
    explicit signal_op_node( const context& context, Args&&... args )
        : signal_op_node::signal_node( context )
        , m_op( std::forward<Args>( args )... )
    {
        this->m_value = evaluate();

        m_op.attach( *this );
    }

    ~signal_op_node() override
    {
        if( !m_was_op_stolen )
        {
            m_op.detach( *this );
        }
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        return this->try_change_value( evaluate() );
    }

    UREACT_WARN_UNUSED_RESULT Op steal_op()
    {
        assert( !m_was_op_stolen && "Op was already stolen" );
        m_was_op_stolen = true;
        m_op.detach( *this );
        return std::move( m_op );
    }

    UREACT_WARN_UNUSED_RESULT bool was_op_stolen() const
    {
        return m_was_op_stolen;
    }

private:
    auto evaluate()
    {
        UREACT_CALLBACK_GUARD( this->get_graph() );
        return m_op.evaluate();
    }

    Op m_op;
    bool m_was_op_stolen = false;
};

} // namespace detail

/*!
 * @brief This signal that exposes additional type information of the linked node, which enables
 * r-value based node merging at construction time
 *
 * The primary use case for this is to avoid unnecessary nodes when creating signal
 * expression from overloaded arithmetic operators.
 *
 * temp_signal shouldn't be used as an l-value type, but instead implicitly
 * converted to signal.
 */
template <typename S, typename Op>
class temp_signal final : public signal<S>
{
private:
    using Node = detail::signal_op_node<S, Op>;

public:
    /*!
     * @brief Construct a fully functional temp signal
     */
    template <typename... Args>
    explicit temp_signal( const context& context, Args&&... args )
        : temp_signal::signal( detail::create_node<Node>( context, std::forward<Args>( args )... ) )
    {}

    /*!
     * @brief Return internal operator, leaving node invalid
     */
    UREACT_WARN_UNUSED_RESULT Op steal_op() &&
    {
        assert( this->get_node_ptr().use_count() == 1
                && "temp_signal's node should be uniquely owned, otherwise it is misused" );
        auto* node_ptr = static_cast<Node*>( this->get_node_ptr().get() );
        return node_ptr->steal_op();
    }

    /*!
     * @brief Checks if internal operator was already stolen
     */
    UREACT_WARN_UNUSED_RESULT bool was_op_stolen() const
    {
        auto* node_ptr = static_cast<Node*>( this->get_node_ptr().get() );
        return node_ptr->was_op_stolen();
    }
};

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_TEMP_SIGNAL_HPP
