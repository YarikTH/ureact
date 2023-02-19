//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_LINKER_FUNCTOR_HPP
#define UREACT_DETAIL_LINKER_FUNCTOR_HPP

#include <memory>

#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

class react_graph;

template <typename Node>
class linker_functor_base
{
protected:
    explicit linker_functor_base( Node& node )
        : m_node( node )
        , m_graph( m_node.get_graph() )
    {}

    Node& m_node;
    react_graph& m_graph;
};

template <typename Node>
class attach_functor : linker_functor_base<Node>
{
public:
    explicit attach_functor( Node& node )
        : linker_functor_base<Node>( node )
    {}

    template <typename... Deps>
    void operator()( const Deps&... deps ) const
    {
        ( attach( deps ), ... );
    }

private:
    template <typename T>
    void attach( const T& op ) const
    {
        op.template attach_rec<Node>( *this );
    }

    template <typename T>
    void attach( const std::shared_ptr<T>& dep_ptr ) const
    {
        this->m_graph.attach_node( this->m_node.get_node_id(), dep_ptr->get_node_id() );
    }
};

template <typename Node>
class detach_functor : linker_functor_base<Node>
{
public:
    explicit detach_functor( Node& node )
        : linker_functor_base<Node>( node )
    {}

    template <typename... Deps>
    void operator()( const Deps&... deps ) const
    {
        ( detach( deps ), ... );
    }

private:
    template <typename T>
    void detach( const T& op ) const
    {
        op.template detach_rec<Node>( *this );
    }

    template <typename T>
    void detach( const std::shared_ptr<T>& dep_ptr ) const
    {
        this->m_graph.detach_node( this->m_node.get_node_id(), dep_ptr->get_node_id() );
    }
};

} // namespace detail

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_LINKER_FUNCTOR_HPP
