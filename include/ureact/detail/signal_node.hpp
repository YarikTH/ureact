#pragma once

#include <cassert>
#include <utility>
#include <tuple>

#include "ureact/detail/reactive_node_interface.hpp"
#include "ureact/detail/context.hpp"
#include "ureact/detail/observer_base.hpp"
#include "ureact/detail/equals.hpp"
#include "ureact/detail/apply.hpp"

namespace ureact
{
namespace detail
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// node_base
///////////////////////////////////////////////////////////////////////////////////////////////////
class node_base : public reactive_node
{
public:
    explicit node_base( context& context )
        : m_context( context )
    {}

    // Nodes can't be copied
    node_base( const node_base& ) = delete;
    node_base& operator=( const node_base& ) = delete;
    node_base( node_base&& ) noexcept = delete;
    node_base& operator=( node_base&& ) noexcept = delete;

    ~node_base() override = default;

    context& get_context() const
    {
        return m_context;
    }

    react_graph& get_graph()
    {
        return _get_internals( m_context ).get_graph();
    }

    const react_graph& get_graph() const
    {
        return _get_internals( m_context ).get_graph();
    }

private:
    context& m_context;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// observer_node
///////////////////////////////////////////////////////////////////////////////////////////////////
class observer_node
    : public node_base
    , public observer_interface
{
public:
    explicit observer_node( context& context )
        : node_base( context )
    {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// observable_node
///////////////////////////////////////////////////////////////////////////////////////////////////
class observable_node
    : public node_base
    , public observable
{
public:
    explicit observable_node( context& context )
        : node_base( context )
    {}
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class signal_node : public observable_node
{
public:
    explicit signal_node( context& context )
        : observable_node( context )
    {}

    template <typename T>
    signal_node( context& context, T&& value )
        : observable_node( context )
        , m_value( std::forward<T>( value ) )
    {}

    const S& value_ref() const
    {
        return m_value;
    }

protected:
    S m_value;
};

template <typename S>
using signal_node_ptr_t = std::shared_ptr<signal_node<S>>;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// var_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class var_node
    : public signal_node<S>
    , public input_node_interface
{
public:
    template <typename T>
    explicit var_node( context& context, T&& value )
        : var_node::signal_node( context, std::forward<T>( value ) )
        , m_new_value( value )
    {}

    var_node( const var_node& ) = delete;
    var_node& operator=( const var_node& ) = delete;
    var_node( var_node&& ) noexcept = delete;
    var_node& operator=( var_node&& ) noexcept = delete;

    // LCOV_EXCL_START
    void tick() override
    {
        assert( false && "Ticked var_node" );
    }
    // LCOV_EXCL_STOP

    template <typename V>
    void request_add_input( V&& new_value )
    {
        var_node::get_graph().add_input( *this, std::forward<V>( new_value ) );
    }

    template <typename F>
    void request_modify_input( F& func )
    {
        var_node::get_graph().modify_input( *this, std::forward<F>( func ) );
    }

    template <typename V>
    void add_input( V&& new_value )
    {
        m_new_value = std::forward<V>( new_value );

        m_is_input_added = true;

        // m_is_input_added takes precedences over m_is_input_modified
        // the only difference between the two is that m_is_input_modified doesn't/can't compare
        m_is_input_modified = false;
    }

    // This is signal-specific
    template <typename F>
    void modify_input( F& func )
    {
        // There hasn't been any set(...) input yet, modify.
        if( !m_is_input_added )
        {
            func( this->m_value );

            m_is_input_modified = true;
        }
        // There's a new_value, modify new_value instead.
        // The modified new_value will handled like before, i.e. it'll be compared to m_value
        // in apply_input
        else
        {
            func( m_new_value );
        }
    }

    bool apply_input() override
    {
        if( m_is_input_added )
        {
            m_is_input_added = false;

            if( !equals( this->m_value, m_new_value ) )
            {
                this->m_value = std::move( m_new_value );
                var_node::get_graph().on_input_change( *this );
                return true;
            }
            return false;
        }
        if( m_is_input_modified )
        {
            m_is_input_modified = false;

            var_node::get_graph().on_input_change( *this );
            return true;
        }
        return false;
    }

private:
    S m_new_value;
    bool m_is_input_added = false;
    bool m_is_input_modified = false;
};

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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_op_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S, typename op_t>
class signal_op_node : public signal_node<S>
{
public:
    template <typename... args_t>
    explicit signal_op_node( context& context, args_t&&... args )
        : signal_op_node::signal_node( context )
        , m_op( std::forward<args_t>( args )... )
    {
        this->m_value = m_op.evaluate();

        m_op.attach( *this );
    }

    signal_op_node( const signal_op_node& ) = delete;
    signal_op_node& operator=( const signal_op_node& ) = delete;
    signal_op_node( signal_op_node&& ) noexcept = delete;
    signal_op_node& operator=( signal_op_node&& ) noexcept = delete;

    ~signal_op_node() override
    {
        if( !m_was_op_stolen )
        {
            m_op.detach( *this );
        }
    }

    void tick() override
    {
        bool changed = false;

        { // timer
            S new_value = m_op.evaluate();

            if( !equals( this->m_value, new_value ) )
            {
                this->m_value = std::move( new_value );
                changed = true;
            }
        } // ~timer

        if( changed )
        {
            signal_op_node::get_graph().on_node_pulse( *this );
        }
    }

    op_t steal_op()
    {
        assert( !m_was_op_stolen && "Op was already stolen." );
        m_was_op_stolen = true;
        m_op.detach( *this );
        return std::move( m_op );
    }

private:
    op_t m_op;
    bool m_was_op_stolen = false;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// flatten_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename outer_t, typename inner_t>
class flatten_node : public signal_node<inner_t>
{
public:
    flatten_node( context& context,
        std::shared_ptr<signal_node<outer_t>> outer,
        const std::shared_ptr<signal_node<inner_t>>& inner )
        : flatten_node::signal_node( context, inner->value_ref() )
        , m_outer( std::move( outer ) )
        , m_inner( inner )
    {
        flatten_node::get_graph().on_node_attach( *this, *m_outer );
        flatten_node::get_graph().on_node_attach( *this, *m_inner );
    }

    ~flatten_node() override
    {
        flatten_node::get_graph().on_node_detach( *this, *m_inner );
        flatten_node::get_graph().on_node_detach( *this, *m_outer );
    }

    // Nodes can't be copied
    flatten_node( const flatten_node& ) = delete;
    flatten_node& operator=( const flatten_node& ) = delete;
    flatten_node( flatten_node&& ) noexcept = delete;
    flatten_node& operator=( flatten_node&& ) noexcept = delete;

    void tick() override
    {
        const auto& new_inner = get_node_ptr( m_outer->value_ref() );

        if( new_inner != m_inner )
        {
            // Topology has been changed
            auto old_inner = m_inner;
            m_inner = new_inner;

            flatten_node::get_graph().on_dynamic_node_detach( *this, *old_inner );
            flatten_node::get_graph().on_dynamic_node_attach( *this, *new_inner );

            return;
        }

        if( !equals( this->m_value, m_inner->value_ref() ) )
        {
            this->m_value = m_inner->value_ref();
            flatten_node::get_graph().on_node_pulse( *this );
        }
    }

private:
    std::shared_ptr<signal_node<outer_t>> m_outer;
    std::shared_ptr<signal_node<inner_t>> m_inner;
};

/// Observer functions can return values of this type to control further processing.
enum class observer_action
{
    next,           ///< Need to continue observing
    stop_and_detach ///< Need to stop observing
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_observer_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S, typename func_t>
class signal_observer_node : public observer_node
{
public:
    template <typename F>
    signal_observer_node(
        context& context, const std::shared_ptr<signal_node<S>>& subject, F&& func )
        : signal_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<F>( func ) )
    {
        get_graph().on_node_attach( *this, *subject );
    }

    signal_observer_node( const signal_observer_node& ) = delete;
    signal_observer_node& operator=( const signal_observer_node& ) = delete;
    signal_observer_node( signal_observer_node&& ) noexcept = delete;
    signal_observer_node& operator=( signal_observer_node&& ) noexcept = delete;

    void tick() override
    {
        bool should_detach = false;

        if( auto p = m_subject.lock() )
        {
            if( m_func( p->value_ref() ) == observer_action::stop_and_detach )
            {
                should_detach = true;
            }
        }

        if( should_detach )
        {
            get_graph().queue_observer_for_detach( *this );
        }
    }

    void unregister_self() override
    {
        if( auto p = m_subject.lock() )
        {
            p->unregister_observer( this );
        }
    }

private:
    void detach_observer() override
    {
        if( auto p = m_subject.lock() )
        {
            get_graph().on_node_detach( *this, *p );
            m_subject.reset();
        }
    }

    std::weak_ptr<signal_node<S>> m_subject;
    func_t m_func;
};

} // namespace detail
} // namespace ureact
