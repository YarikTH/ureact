#pragma once

#include <tuple>

#include "ureact/detail/cpp_future.hpp"

namespace ureact
{
namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// attach/detach helper functors
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename node_t, typename... deps_t>
struct attach_functor
{
    explicit attach_functor( node_t& node )
        : my_node( node )
    {}

    void operator()( const deps_t&... deps ) const
    {
        REACT_EXPAND_PACK( attach( deps ) );
    }

    template <typename T>
    void attach( const T& op ) const
    {
        op.template attach_rec<node_t>( *this );
    }

    template <typename T>
    void attach( const std::shared_ptr<T>& dep_ptr ) const
    {
        my_node.get_context()->on_node_attach( my_node, *dep_ptr );
    }

    node_t& my_node;
};

template <typename node_t, typename... deps_t>
struct detach_functor
{
    explicit detach_functor( node_t& node )
        : my_node( node )
    {}

    void operator()( const deps_t&... deps ) const
    {
        REACT_EXPAND_PACK( detach( deps ) );
    }

    template <typename T>
    void detach( const T& op ) const
    {
        op.template detach_rec<node_t>( *this );
    }

    template <typename T>
    void detach( const std::shared_ptr<T>& dep_ptr ) const
    {
        my_node.get_context()->on_node_detach( my_node, *dep_ptr );
    }

    node_t& my_node;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// reactive_op_base
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... deps_t>
class reactive_op_base
{
public:
    using dep_holder_t = std::tuple<deps_t...>;

    template <typename... deps_in_t>
    explicit reactive_op_base( dont_move /*unused*/, deps_in_t&&... deps )
        : m_deps( std::forward<deps_in_t>( deps )... )
    {}

    ~reactive_op_base() = default;

    reactive_op_base( reactive_op_base&& other ) noexcept
        : m_deps( std::move( other.m_deps ) )
    {}

    reactive_op_base& operator=( reactive_op_base&& ) noexcept = delete;

    // Can't be copied, only moved
    reactive_op_base( const reactive_op_base& other ) = delete;
    reactive_op_base& operator=( const reactive_op_base& ) = delete;

    template <typename node_t>
    void attach( node_t& node ) const
    {
        apply( attach_functor<node_t, deps_t...>{ node }, m_deps );
    }

    template <typename node_t>
    void detach( node_t& node ) const
    {
        apply( detach_functor<node_t, deps_t...>{ node }, m_deps );
    }

    template <typename node_t, typename functor_t>
    void attach_rec( const functor_t& functor ) const
    {
        // Same memory layout, different func
        apply( reinterpret_cast<const attach_functor<node_t, deps_t...>&>( functor ), m_deps );
    }

    template <typename node_t, typename functor_t>
    void detach_rec( const functor_t& functor ) const
    {
        apply( reinterpret_cast<const detach_functor<node_t, deps_t...>&>( functor ), m_deps );
    }

protected:
    dep_holder_t m_deps;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// function_op
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S, typename F, typename... deps_t>
class function_op : public reactive_op_base<deps_t...>
{
public:
    template <typename in_f, typename... deps_in_t>
    explicit function_op( in_f&& func, deps_in_t&&... deps )
        : function_op::reactive_op_base( dont_move(), std::forward<deps_in_t>( deps )... )
        , m_func( std::forward<in_f>( func ) )
    {}

    function_op( function_op&& other ) noexcept
        : function_op::reactive_op_base( std::move( other ) )
        , m_func( std::move( other.m_func ) )
    {}

    function_op& operator=( function_op&& ) noexcept = delete;

    function_op( const function_op& ) = delete;
    function_op& operator=( const function_op& ) = delete;

    ~function_op() = default;

    S evaluate()
    {
        return apply( eval_functor( m_func ), this->m_deps );
    }

private:
    // Eval
    struct eval_functor
    {
        explicit eval_functor( F& f )
            : my_func( f )
        {}

        template <typename... T>
        S operator()( T&&... args )
        {
            return my_func( eval( args )... );
        }

        template <typename T>
        static auto eval( T& op ) -> decltype( op.evaluate() )
        {
            return op.evaluate();
        }

        template <typename T>
        static auto eval( const std::shared_ptr<T>& dep_ptr ) -> decltype( dep_ptr->value_ref() )
        {
            return dep_ptr->value_ref();
        }

        F& my_func;
    };

    F m_func;
};

} // namespace detail
} // namespace ureact
