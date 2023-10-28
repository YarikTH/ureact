//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_NODE_HPP
#define UREACT_DETAIL_NODE_HPP

#include <memory>
#include <tuple>
#include <utility>

#include <ureact/context.hpp>
#include <ureact/detail/graph_impl.hpp>
#include <ureact/detail/graph_interface.hpp>
#include <ureact/detail/node_id_vector.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename Node, typename... Args>
auto create_node( Args&&... args )
{
    auto result = std::make_shared<Node>( std::forward<Args>( args )... );
    node_id id = result->get_node_id();
    react_graph& graph = get_internals( result->get_context() ).get_graph();
    graph.register_node_ptr( id, result );
    return result;
}

template <typename Ret, typename Node, typename... Args>
Ret create_wrapped_node( Args&&... args )
{
    return Ret{ create_node<Node>( std::forward<Args>( args )... ) };
}

class UREACT_API node : public reactive_node_interface
{
public:
    explicit node( context context );

    ~node() override;

    UREACT_WARN_UNUSED_RESULT node_id get_node_id() const
    {
        return m_id;
    }

    UREACT_WARN_UNUSED_RESULT context& get_context()
    {
        return m_context;
    }

    UREACT_WARN_UNUSED_RESULT const context& get_context() const
    {
        return m_context;
    }

protected:
    UREACT_WARN_UNUSED_RESULT react_graph& get_graph();
    UREACT_WARN_UNUSED_RESULT const react_graph& get_graph() const;

    void attach_to( node_id parentId );

    void detach_from( node_id parentId );

    void detach_from_all();

    template <class... Deps>
    void attach_to( const Deps&... deps )
    {
        ( attach_to( get_internals( deps ).get_node_id() ), ... );
    }

    template <class... Deps>
    void attach_to( const std::tuple<Deps...>& tp )
    {
        std::apply(
            [this]( const auto&... deps ) { //
                this->attach_to( deps... );
            },
            tp );
    }

    template <typename Node>
    friend class attach_functor;

    template <typename Node>
    friend class detach_functor;

private:
    UREACT_MAKE_NONCOPYABLE( node );

    context m_context{};

    node_id m_id;

    node_id_vector m_parents;
};

} // namespace detail

UREACT_END_NAMESPACE

#if UREACT_HEADER_ONLY
#    include <ureact/detail/node.inl>
#endif

#endif // UREACT_DETAIL_NODE_HPP
