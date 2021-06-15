#pragma once

#include <tuple>

#include "ureact/detail/apply.hpp"

namespace ureact
{
namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper to enable calling a function on each element of an argument pack.
/// We can't do f(args) ...; because ... expands with a comma.
/// But we can do nop_func(f(args) ...);
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... args_t>
inline void pass( args_t&&... /*unused*/ )
{}

// Expand args by wrapping them in a dummy function
// Use comma operator to replace potential void return value with 0
#define UREACT_EXPAND_PACK( ... ) pass( ( __VA_ARGS__, 0 )... )

///////////////////////////////////////////////////////////////////////////////////////////////////
/// function_op
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S, typename F, typename... deps_t>
class function_op
{
public:
    using dep_holder_t = std::tuple<deps_t...>;

    template <typename in_f, typename... deps_in_t>
    explicit function_op( in_f&& func, deps_in_t&&... deps )
        : m_deps( std::forward<deps_in_t>( deps )... )
        , m_func( std::forward<in_f>( func ) )
    {}

    function_op( function_op&& other ) noexcept
        : m_deps( std::move( other.m_deps ) )
        , m_func( std::move( other.m_func ) )
    {}

    function_op& operator=( function_op&& ) noexcept = delete;

    function_op( const function_op& ) = delete;
    function_op& operator=( const function_op& ) = delete;

    ~function_op() = default;

    S evaluate()
    {
        return apply( eval_functor( m_func ), m_deps );
    }

    template <typename node_t>
    void attach( node_t& node ) const
    {
        apply( attach_functor<node_t>{ node }, m_deps );
    }

    template <typename node_t>
    void detach( node_t& node ) const
    {
        apply( detach_functor<node_t>{ node }, m_deps );
    }

    template <typename node_t, typename functor_t>
    void attach_rec( const functor_t& functor ) const
    {
        // Same memory layout, different func
        apply( reinterpret_cast<const attach_functor<node_t>&>( functor ), m_deps );
    }

    template <typename node_t, typename functor_t>
    void detach_rec( const functor_t& functor ) const
    {
        apply( reinterpret_cast<const detach_functor<node_t>&>( functor ), m_deps );
    }

private:
    template <typename node_t>
    struct attach_functor
    {
        explicit attach_functor( node_t& node )
            : node( node )
        {}

        void operator()( const deps_t&... deps ) const
        {
            UREACT_EXPAND_PACK( attach( deps ) );
        }

        template <typename T>
        void attach( const T& op ) const
        {
            op.template attach_rec<node_t>( *this );
        }

        template <typename T>
        void attach( const std::shared_ptr<T>& dep_ptr ) const
        {
            node.get_graph().on_node_attach( node, *dep_ptr );
        }

        node_t& node;
    };

    template <typename node_t>
    struct detach_functor
    {
        explicit detach_functor( node_t& node )
            : node( node )
        {}

        void operator()( const deps_t&... deps ) const
        {
            UREACT_EXPAND_PACK( detach( deps ) );
        }

        template <typename T>
        void detach( const T& op ) const
        {
            op.template detach_rec<node_t>( *this );
        }

        template <typename T>
        void detach( const std::shared_ptr<T>& dep_ptr ) const
        {
            node.get_graph().on_node_detach( node, *dep_ptr );
        }

        node_t& node;
    };

    // Eval
    struct eval_functor
    {
        explicit eval_functor( F& f )
            : func( f )
        {}

        template <typename... T>
        S operator()( T&&... args )
        {
            return func( eval( args )... );
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

        F& func;
    };

    dep_holder_t m_deps;
    F m_func;
};

#undef UREACT_EXPAND_PACK

} // namespace detail
} // namespace ureact
