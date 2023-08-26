//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_GRAPH_IMPL_HPP
#define UREACT_DETAIL_GRAPH_IMPL_HPP

#include <memory>

#include <ureact/detail/defines.hpp>
#include <ureact/detail/graph_interface.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

#if !defined( NDEBUG )
struct callback_sanitizer
{
    virtual ~callback_sanitizer() = default;

    /// Return if external callback is in progress
    UREACT_WARN_UNUSED_RESULT virtual bool is_locked() const = 0;

    /// Marks begin of an external callback
    virtual void begin_external_callback() = 0;

    /// Marks end of an external callback
    virtual void end_external_callback() = 0;

    /// Marks a place where an external callback is called
    struct guard
    {
        callback_sanitizer& self;

        explicit guard( callback_sanitizer& self )
            : self( self )
        {
            self.begin_external_callback();
        }

        ~guard()
        {
            self.end_external_callback();
        }

        UREACT_MAKE_NONCOPYABLE( guard );
        UREACT_MAKE_NONMOVABLE( guard );
    };
};
#    define UREACT_CALLBACK_GUARD( _SELF_ ) callback_sanitizer::guard _( _SELF_ )
#else
#    define UREACT_CALLBACK_GUARD( _SELF_ )                                                        \
        do                                                                                         \
        {                                                                                          \
        } while( false )
#endif

struct react_graph
#if !defined( NDEBUG )
    : public callback_sanitizer
#endif
{
    virtual ~react_graph() = default;

    virtual node_id register_node() = 0;
    virtual void register_node_ptr(
        node_id nodeId, const std::weak_ptr<reactive_node_interface>& nodePtr )
        = 0;
    virtual void unregister_node( node_id nodeId ) = 0;

    virtual void attach_node( node_id nodeId, node_id parentId ) = 0;
    virtual void detach_node( node_id nodeId, node_id parentId ) = 0;

    virtual void push_input( node_id nodeId ) = 0;

    virtual void start_transaction() = 0;
    virtual void finish_transaction() = 0;

    [[nodiscard]] virtual bool is_propagation_in_progress() const = 0;
};

std::shared_ptr<react_graph> make_react_graph();

} // namespace detail

UREACT_END_NAMESPACE

#if UREACT_HEADER_ONLY
#    include <ureact/detail/graph_impl.inl>
#endif

#endif // UREACT_DETAIL_GRAPH_IMPL_HPP
