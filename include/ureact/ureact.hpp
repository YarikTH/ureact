// ureact.hpp - minimalistic C++ single-header reactive library
//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2021 Krylov Yaroslav.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// The library is heavily influenced by cpp.react - https://github.com/snakster/cpp.react
// which uses the Boost Software License - Version 1.0
// see here - https://github.com/snakster/cpp.react/blob/master/LICENSE_1_0.txt
// Project started as a complex refactoring and transformation of cpp.react's codebase
// but with different design goals.
//
// The documentation can be found at the library's page:
// https://github.com/YarikTH/ureact/blob/main/README.md
//

#ifndef UREACT_UREACT_H_
#define UREACT_UREACT_H_

#define UREACT_VERSION_MAJOR 0
#define UREACT_VERSION_MINOR 5
#define UREACT_VERSION_PATCH 0
#define UREACT_VERSION_STR "0.5.0"

#define UREACT_VERSION                                                                             \
    ( UREACT_VERSION_MAJOR * 10000 + UREACT_VERSION_MINOR * 100 + UREACT_VERSION_PATCH )

#include <cassert>
#include <limits>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef UREACT_USE_STD_ALGORITHM
#    include <algorithm>
#endif

//==================================================================================================
// [[section]] Preprocessor feature detections
// Mostly based on https://github.com/fmtlib/fmt/blob/master/include/fmt/core.h
// and on https://github.com/nemequ/hedley
//==================================================================================================

static_assert( __cplusplus >= 201703L, "At least c++17 standard is required" );

#ifdef __clang__
#    define UREACT_CLANG_VERSION ( __clang_major__ * 100 + __clang_minor__ )
#else
#    define UREACT_CLANG_VERSION 0
#endif

#if defined( __GNUC__ ) && !defined( __clang__ )
#    define UREACT_GCC_VERSION ( __GNUC__ * 100 + __GNUC_MINOR__ )
#    define UREACT_GCC_PRAGMA( arg ) _Pragma( arg )
#else
#    define UREACT_GCC_VERSION 0
#    define UREACT_GCC_PRAGMA( arg )
#endif

#ifdef _MSC_VER
#    define UREACT_MSC_VER _MSC_VER
#    define UREACT_MSC_WARNING( ... ) __pragma( warning( __VA_ARGS__ ) )
#else
#    define UREACT_MSC_VER 0
#    define UREACT_MSC_WARNING( ... )
#endif

#ifdef __has_feature
#    define UREACT_HAS_FEATURE( x ) __has_feature( x )
#else
#    define UREACT_HAS_FEATURE( x ) 0
#endif

#ifdef __has_cpp_attribute
#    define UREACT_HAS_CPP_ATTRIBUTE( x ) __has_cpp_attribute( x )
#else
#    define UREACT_HAS_CPP_ATTRIBUTE( x ) 0
#endif

#define UREACT_HAS_CPP14_ATTRIBUTE( attribute )                                                    \
    ( __cplusplus >= 201402L && UREACT_HAS_CPP_ATTRIBUTE( attribute ) )

#define UREACT_HAS_CPP17_ATTRIBUTE( attribute )                                                    \
    ( __cplusplus >= 201703L && UREACT_HAS_CPP_ATTRIBUTE( attribute ) )

#if( UREACT_MSC_VER )
// MSC erroneously warns about discarding return value when it is not discarded.
// It seems that it doesn't understand that comma operator can be overloaded.
#    define UREACT_WARN_UNUSED_RESULT
#    define UREACT_WARN_UNUSED_RESULT_MSG( msg )
#elif( UREACT_HAS_CPP17_ATTRIBUTE( nodiscard ) >= 201907L )
#    define UREACT_WARN_UNUSED_RESULT [[nodiscard]]
#    define UREACT_WARN_UNUSED_RESULT_MSG( msg ) [[nodiscard( msg )]]
#elif UREACT_HAS_CPP17_ATTRIBUTE( nodiscard )
#    define UREACT_WARN_UNUSED_RESULT [[nodiscard]]
#    define UREACT_WARN_UNUSED_RESULT_MSG( msg ) [[nodiscard]]
#elif defined( _Check_return_ ) /* SAL */
#    define UREACT_WARN_UNUSED_RESULT _Check_return_
#    define UREACT_WARN_UNUSED_RESULT_MSG( msg ) _Check_return_
#else
#    define UREACT_WARN_UNUSED_RESULT
#    define UREACT_WARN_UNUSED_RESULT_MSG( msg )
#endif

#ifndef UREACT_DEPRECATED
#    if UREACT_HAS_CPP14_ATTRIBUTE( deprecated ) || UREACT_MSC_VER >= 1900
#        define UREACT_DEPRECATED [[deprecated]]
#    else
#        if( defined( __GNUC__ ) && !defined( __LCC__ ) ) || defined( __clang__ )
#            define UREACT_DEPRECATED __attribute__( ( deprecated ) )
#        elif UREACT_MSC_VER
#            define UREACT_DEPRECATED __declspec( deprecated )
#        else
#            define UREACT_DEPRECATED /* deprecated */
#        endif
#    endif
#endif

#ifndef UREACT_USE_INLINE_NAMESPACES
#    if UREACT_HAS_FEATURE( cxx_inline_namespaces ) || UREACT_GCC_VERSION >= 404                   \
        || ( UREACT_MSC_VER >= 1900 && ( !defined( _MANAGED ) || !_MANAGED ) )
#        define UREACT_USE_INLINE_NAMESPACES 1
#    else
#        define UREACT_USE_INLINE_NAMESPACES 0
#    endif
#endif

#define UREACT_VERSION_NAMESPACE_NAME v0

#ifndef UREACT_BEGIN_NAMESPACE
#    if UREACT_USE_INLINE_NAMESPACES
#        define UREACT_INLINE_NAMESPACE inline namespace
#        define UREACT_END_NAMESPACE                                                               \
            } /* namespace ureact */                                                               \
            } /* namespace UREACT_VERSION_NAMESPACE_NAME */
#    else
#        define UREACT_INLINE_NAMESPACE namespace
#        define UREACT_END_NAMESPACE                                                               \
            }                                                                                      \
            using namespace UREACT_VERSION_NAMESPACE_NAME;                                         \
            }
#    endif
#    define UREACT_BEGIN_NAMESPACE                                                                 \
        namespace ureact                                                                           \
        {                                                                                          \
        UREACT_INLINE_NAMESPACE UREACT_VERSION_NAMESPACE_NAME                                      \
        {
#endif

UREACT_BEGIN_NAMESPACE

//==================================================================================================
// [[section]] Forward declarations
//==================================================================================================
class context;

template <typename S>
class signal;

template <typename S>
class function;

template <typename S>
class value;


namespace detail
{

class context_internals;

} // namespace detail

/// Return internals. Not intended to use in user code.
UREACT_WARN_UNUSED_RESULT inline detail::context_internals& _get_internals( context& ctx );

/// Observer functions can return values of this type to control further processing.
enum class observer_action
{
    next,           ///< Need to continue observing
    stop_and_detach ///< Need to stop observing
};

namespace detail
{

// Got from https://stackoverflow.com/a/34672753
// std::is_base_of for template classes
template <template <typename...> class base, typename derived>
struct is_base_of_template_impl
{
    template <typename... Ts>
    static constexpr std::true_type test( const base<Ts...>* )
    {
        return {};
    }
    static constexpr std::false_type test( ... )
    {
        return {};
    }
    using type = decltype( test( std::declval<derived*>() ) );
};

template <template <typename...> class base, typename derived>
using is_base_of_template = typename is_base_of_template_impl<base, derived>::type;

} // namespace detail

/// Return if type is signal or its inheritor
template <typename T>
struct is_signal : detail::is_base_of_template<signal, T>
{};

template <typename T>
inline constexpr bool is_signal_v = is_signal<T>::value;

//==================================================================================================
// [[section]] General purpose utilities
//==================================================================================================
namespace detail
{

#if defined( UREACT_USE_STD_ALGORITHM )

using std::find;
using std::partition;

#else

// Partial alternative to <algorithm> is provided and used by default because library requires
// only a few algorithms while standard <algorithm> is quite bloated

// Code based on possible implementation at
// https://en.cppreference.com/w/cpp/algorithm/find
template <typename forward_it, typename T>
inline forward_it find( forward_it first, forward_it last, const T& val )
{
    for( auto it = first, ite = last; it != ite; ++it )
    {
        if( *it == val )
        {
            return it;
        }
    }
    return last;
}

// Code based on possible implementation at
// https://en.cppreference.com/w/cpp/algorithm/find
template <class forward_it, class unary_predicate>
forward_it find_if_not( forward_it first, forward_it last, unary_predicate q )
{
    for( ; first != last; ++first )
    {
        if( !q( *first ) )
        {
            return first;
        }
    }
    return last;
}

// Code based on possible implementation at
// https://en.cppreference.com/w/cpp/algorithm/iter_swap
template <class forward_it_1, class forward_it_2>
void iter_swap( forward_it_1 a, forward_it_2 b )
{
    using std::swap;
    swap( *a, *b );
}

// Code based on possible implementation at
// https://en.cppreference.com/w/cpp/algorithm/partition
template <class forward_it, class unary_predicate>
forward_it partition( forward_it first, forward_it last, unary_predicate p )
{
    first = ureact::detail::find_if_not( first, last, p );
    if( first == last )
    {
        return first;
    }

    for( forward_it i = std::next( first ); i != last; ++i )
    {
        if( p( *i ) )
        {
            ureact::detail::iter_swap( i, first );
            ++first;
        }
    }
    return first;
}

#endif


template <typename T1, typename T2>
using is_same_decay = std::is_same<std::decay_t<T1>, std::decay_t<T2>>;

template <typename T1, typename T2>
inline constexpr bool is_same_decay_v = is_same_decay<T1, T2>::value;


/// type_identity (workaround to enable enable decltype()::X)
/// See https://en.cppreference.com/w/cpp/types/type_identity
template <typename T>
struct type_identity
{
    using type = T;
};

template <typename T>
using type_identity_t = typename type_identity<T>::type;


// Full analog of std::bind1st that removed in c++17
// See https://en.cppreference.com/w/cpp/utility/functional/bind12
template <template <typename, typename> class functor_binary_op,
    typename lhs_t,
    typename rhs_t,
    typename F = functor_binary_op<lhs_t, rhs_t>>
struct bind_left
{
    bind_left( bind_left&& other ) noexcept = default;

    bind_left& operator=( bind_left&& other ) noexcept = delete;

    template <typename T, class = std::enable_if_t<!is_same_decay_v<T, bind_left>>>
    explicit bind_left( T&& val )
        : m_left_val( std::forward<T>( val ) )
    {}

    bind_left( const bind_left& other ) = delete;

    bind_left& operator=( const bind_left& other ) = delete;

    ~bind_left() = default;

    UREACT_WARN_UNUSED_RESULT auto operator()( const rhs_t& rhs ) const
    {
        return F()( m_left_val, rhs );
    }

    lhs_t m_left_val;
};


// Full analog of std::bind2nd that removed in c++17
// See https://en.cppreference.com/w/cpp/utility/functional/bind12
template <template <typename, typename> class functor_binary_op,
    typename lhs_t,
    typename rhs_t,
    typename F = functor_binary_op<lhs_t, rhs_t>>
struct bind_right
{
    bind_right( bind_right&& other ) noexcept = default;

    bind_right& operator=( bind_right&& other ) noexcept = delete;

    template <typename T, class = std::enable_if_t<!is_same_decay_v<T, bind_right>>>
    explicit bind_right( T&& val )
        : m_right_val( std::forward<T>( val ) )
    {}

    bind_right( const bind_right& other ) = delete;

    bind_right& operator=( const bind_right& other ) = delete;

    ~bind_right() = default;

    UREACT_WARN_UNUSED_RESULT auto operator()( const lhs_t& lhs ) const
    {
        return F()( lhs, m_right_val );
    }

    rhs_t m_right_val;
};


/// Special wrapper to add specific return type to the void function
template <typename F, typename ret_t, ret_t return_value>
struct add_default_return_value_wrapper
{
    add_default_return_value_wrapper( const add_default_return_value_wrapper& ) = default;

    add_default_return_value_wrapper& operator=( const add_default_return_value_wrapper& ) = delete;

    add_default_return_value_wrapper( add_default_return_value_wrapper&& other ) noexcept
        : m_func( std::move( other.m_func ) )
    {}

    add_default_return_value_wrapper& operator=(
        add_default_return_value_wrapper&& ) noexcept = delete;

    template <typename in_f,
        class = std::enable_if_t<!is_same_decay_v<in_f, add_default_return_value_wrapper>>>
    explicit add_default_return_value_wrapper( in_f&& func )
        : m_func( std::forward<in_f>( func ) )
    {}

    ~add_default_return_value_wrapper() = default;

    template <typename... args_t>
    UREACT_WARN_UNUSED_RESULT ret_t operator()( args_t&&... args )
    {
        m_func( std::forward<args_t>( args )... );
        return return_value;
    }

    F m_func;
};

} // namespace detail



//==================================================================================================
// [[section]] Ureact specific utilities
//==================================================================================================
namespace detail
{

template <typename T>
struct decay_input
{
    using type = T;
};

template <typename T>
struct decay_input<value<T>>
{
    using type = signal<T>;
};

template <typename T>
using decay_input_t = typename decay_input<T>::type;


#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wfloat-equal"
#endif

template <typename L, typename R>
UREACT_WARN_UNUSED_RESULT bool equals( const L& lhs, const R& rhs )
{
    return lhs == rhs;
}

template <typename L, typename R>
UREACT_WARN_UNUSED_RESULT bool equals(
    const std::reference_wrapper<L>& lhs, const std::reference_wrapper<R>& rhs )
{
    return lhs.get() == rhs.get();
}

template <typename L, typename R>
UREACT_WARN_UNUSED_RESULT bool equals( const signal<L>& lhs, const signal<R>& rhs )
{
    return lhs.equals( rhs );
}

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic pop
#endif

} // namespace detail



//==================================================================================================
// [[section]] Ureact engine
//==================================================================================================
namespace detail
{

class reactive_node
{
public:
    int level{ 0 };
    int new_level{ 0 };
    bool queued{ false };

    std::vector<reactive_node*> successors;

    virtual ~reactive_node() = default;

    virtual void tick() = 0;
};


struct input_node_interface
{
    virtual ~input_node_interface() = default;

    virtual bool apply_input() = 0;
};


class observer_interface
{
public:
    virtual ~observer_interface() = default;

    virtual void unregister_self() = 0;

private:
    virtual void detach_observer() = 0;

    friend class observable;
};


class observable
{
public:
    observable() = default;

    observable( const observable& ) = delete;
    observable& operator=( const observable& ) = delete;
    observable( observable&& ) noexcept = delete;
    observable& operator=( observable&& ) noexcept = delete;

    ~observable()
    {
        for( const auto& p : m_observers )
            if( p != nullptr )
                p->detach_observer();
    }

    void register_observer( std::unique_ptr<observer_interface>&& obs_ptr )
    {
        m_observers.push_back( std::move( obs_ptr ) );
    }

    void unregister_observer( observer_interface* raw_obs_ptr )
    {
        for( auto it = m_observers.begin(); it != m_observers.end(); ++it )
        {
            if( it->get() == raw_obs_ptr )
            {
                it->get()->detach_observer();
                m_observers.erase( it );
                break;
            }
        }
    }

private:
    std::vector<std::unique_ptr<observer_interface>> m_observers;
};


class react_graph
{
public:
    react_graph() = default;

    template <typename F>
    void do_transaction( F&& func )
    {
        ++m_transaction_level;
        func();
        --m_transaction_level;

        if( m_transaction_level == 0 )
        {
            finalize_transaction();
        }
    }

    void finalize_transaction()
    {
        // apply input node changes
        bool should_propagate = false;
        for( auto* p : m_changed_inputs )
        {
            if( p->apply_input() )
            {
                should_propagate = true;
            }
        }
        m_changed_inputs.clear();

        // propagate changes
        if( should_propagate )
        {
            propagate();
        }

        detach_queued_observers();
    }

    template <typename F>
    void push_input( input_node_interface* node, F&& inputCallback )
    {
        m_changed_inputs.push_back( node );

        // This writes to the input buffer of the respective node.
        std::forward<F>( inputCallback )();

        if( m_transaction_level == 0 )
        {
            finalize_transaction();
        }
    }

    void queue_observer_for_detach( observer_interface& obs )
    {
        m_detached_observers.push_back( &obs );
    }

    void propagate();

    void on_node_attach( reactive_node& node, reactive_node& parent );
    void on_node_detach( reactive_node& node, reactive_node& parent );

    void on_input_change( reactive_node& node );
    void on_node_pulse( reactive_node& node );

    void on_dynamic_node_attach( reactive_node& node, reactive_node& parent );
    void on_dynamic_node_detach( reactive_node& node, reactive_node& parent );

private:
    class topological_queue
    {
    public:
        using value_type = reactive_node*;

        topological_queue() = default;

        void push( const value_type& value, const int level )
        {
            m_queue_data.emplace_back( value, level );
        }

        bool fetch_next();

        const std::vector<value_type>& next_values() const
        {
            return m_next_data;
        }

    private:
        using entry = std::pair<value_type, int>;

        std::vector<value_type> m_next_data;
        std::vector<entry> m_queue_data;
    };

    void detach_queued_observers()
    {
        for( auto* o : m_detached_observers )
        {
            o->unregister_self();
        }
        m_detached_observers.clear();
    }

    static void recalculate_successor_levels( reactive_node& node );

    void process_children( reactive_node& node );

    topological_queue m_scheduled_nodes;

    int m_transaction_level = 0;

    std::vector<input_node_interface*> m_changed_inputs;

    std::vector<observer_interface*> m_detached_observers;
};


UREACT_WARN_UNUSED_RESULT inline bool react_graph::topological_queue::fetch_next()
{
    // Throw away previous values
    m_next_data.clear();

    // Find min level of nodes in queue data
    int minimal_level = std::numeric_limits<int>::max();
    for( const auto& e : m_queue_data )
    {
        if( minimal_level > e.second )
        {
            minimal_level = e.second;
        }
    }

    // Swap entries with min level to the end
    const auto p = ureact::detail::partition( m_queue_data.begin(),
        m_queue_data.end(),
        [minimal_level]( const entry& e ) { return e.second != minimal_level; } );

    // Reserve once to avoid multiple re-allocations
    const auto to_reserve = static_cast<size_t>( std::distance( p, m_queue_data.end() ) );
    m_next_data.reserve( to_reserve );

    // Move min level values to next data
    for( auto it = p, ite = m_queue_data.end(); it != ite; ++it )
    {
        m_next_data.push_back( it->first );
    }

    // Truncate moved entries
    const auto to_resize = static_cast<size_t>( std::distance( m_queue_data.begin(), p ) );
    m_queue_data.resize( to_resize );

    return !m_next_data.empty();
}

inline void react_graph::on_node_attach( reactive_node& node, reactive_node& parent )
{
    parent.successors.push_back( &node );

    if( node.level <= parent.level )
    {
        node.level = parent.level + 1;
    }
}

inline void react_graph::on_node_detach( reactive_node& node, reactive_node& parent )
{
    const auto it
        = ureact::detail::find( parent.successors.begin(), parent.successors.end(), &node );
    parent.successors.erase( it );
}

inline void react_graph::on_input_change( reactive_node& node )
{
    process_children( node );
}

inline void react_graph::on_node_pulse( reactive_node& node )
{
    process_children( node );
}

inline void react_graph::propagate()
{
    while( m_scheduled_nodes.fetch_next() )
    {
        for( auto* cur_node : m_scheduled_nodes.next_values() )
        {
            if( cur_node->level < cur_node->new_level )
            {
                cur_node->level = cur_node->new_level;
                recalculate_successor_levels( *cur_node );
                m_scheduled_nodes.push( cur_node, cur_node->level );
                continue;
            }

            cur_node->queued = false;
            cur_node->tick();
        }
    }
}

inline void react_graph::on_dynamic_node_attach( reactive_node& node, reactive_node& parent )
{
    on_node_attach( node, parent );

    recalculate_successor_levels( node );

    // Re-schedule this node
    node.queued = true;
    m_scheduled_nodes.push( &node, node.level );
}

inline void react_graph::on_dynamic_node_detach( reactive_node& node, reactive_node& parent )
{
    on_node_detach( node, parent );
}

inline void react_graph::process_children( reactive_node& node )
{
    // add children to queue
    for( auto* successor : node.successors )
    {
        if( !successor->queued )
        {
            successor->queued = true;
            m_scheduled_nodes.push( successor, successor->level );
        }
    }
}

inline void react_graph::recalculate_successor_levels( reactive_node& node )
{
    for( auto* successor : node.successors )
    {
        if( successor->new_level <= node.level )
        {
            successor->new_level = node.level + 1;
        }
    }
}

class context_internals
{
public:
    context_internals()
        : m_graph( new react_graph() )
    {}

    UREACT_WARN_UNUSED_RESULT react_graph& get_graph()
    {
        return *m_graph;
    }

    UREACT_WARN_UNUSED_RESULT const react_graph& get_graph() const
    {
        return *m_graph;
    }

private:
    std::unique_ptr<react_graph> m_graph;
};

} // namespace detail



//==================================================================================================
// [[section]] Reactive nodes
//==================================================================================================
namespace detail
{

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

    UREACT_WARN_UNUSED_RESULT context& get_context() const
    {
        return m_context;
    }

    UREACT_WARN_UNUSED_RESULT react_graph& get_graph()
    {
        return _get_internals( m_context ).get_graph();
    }

    UREACT_WARN_UNUSED_RESULT const react_graph& get_graph() const
    {
        return _get_internals( m_context ).get_graph();
    }

private:
    context& m_context;
};


class observer_node
    : public node_base
    , public observer_interface
{
public:
    explicit observer_node( context& context )
        : node_base( context )
    {}
};


class observable_node
    : public node_base
    , public observable
{
public:
    explicit observable_node( context& context )
        : node_base( context )
    {}
};


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

    UREACT_WARN_UNUSED_RESULT const S& value_ref() const
    {
        return m_value;
    }

protected:
    S m_value;
};


template <typename S>
using signal_node_ptr_t = std::shared_ptr<signal_node<S>>;


template <typename S>
class value_node
    : public signal_node<S>
    , public input_node_interface
{
public:
    template <typename T>
    explicit value_node( context& context, T&& value )
        : value_node::signal_node( context, std::forward<T>( value ) )
        , m_new_value( value )
    {}

    value_node( const value_node& ) = delete;
    value_node& operator=( const value_node& ) = delete;
    value_node( value_node&& ) noexcept = delete;
    value_node& operator=( value_node&& ) noexcept = delete;

    // LCOV_EXCL_START
    void tick() override
    {
        assert( false && "Ticked value_node" );
    }
    // LCOV_EXCL_STOP

    template <typename V>
    void set_value( V&& new_value )
    {
        m_new_value = std::forward<V>( new_value );

        m_is_input_added = true;

        // m_is_input_added takes precedences over m_is_input_modified
        // the only difference between the two is that m_is_input_modified doesn't/can't compare
        m_is_input_modified = false;
    }

    // This is signal-specific
    template <typename F>
    void modify_value( F& func )
    {
        // There hasn't been any set(...) input yet, modify.
        if( !m_is_input_added )
        {
            func( this->m_value );

            m_is_input_modified = true;
        }
        // There's a new_value, modify new_value instead.
        // The modified new_value will be handled like before, i.e. it'll be compared to m_value
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
                value_node::get_graph().on_input_change( *this );
                return true;
            }
            return false;
        }
        if( m_is_input_modified )
        {
            m_is_input_modified = false;

            value_node::get_graph().on_input_change( *this );
            return true;
        }
        return false;
    }

private:
    S m_new_value;
    bool m_is_input_added = false;
    bool m_is_input_modified = false;
};


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

    UREACT_WARN_UNUSED_RESULT S evaluate()
    {
        return std::apply( eval_functor( m_func ), m_deps );
    }

    template <typename node_t>
    void attach( node_t& node ) const
    {
        std::apply( attach_functor<node_t>{ node }, m_deps );
    }

    template <typename node_t>
    void detach( node_t& node ) const
    {
        std::apply( detach_functor<node_t>{ node }, m_deps );
    }

    template <typename node_t, typename functor_t>
    void attach_rec( const functor_t& functor ) const
    {
        // Same memory layout, different func
        std::apply( reinterpret_cast<const attach_functor<node_t>&>( functor ), m_deps );
    }

    template <typename node_t, typename functor_t>
    void detach_rec( const functor_t& functor ) const
    {
        std::apply( reinterpret_cast<const detach_functor<node_t>&>( functor ), m_deps );
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
            ( attach( deps ), ... );
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
            ( detach( deps ), ... );
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
        UREACT_WARN_UNUSED_RESULT S operator()( T&&... args )
        {
            return func( eval( args )... );
        }

        template <typename T>
        UREACT_WARN_UNUSED_RESULT static auto eval( T& op ) -> decltype( op.evaluate() )
        {
            return op.evaluate();
        }

        template <typename T>
        UREACT_WARN_UNUSED_RESULT static auto eval( const std::shared_ptr<T>& dep_ptr )
            -> decltype( dep_ptr->value_ref() )
        {
            return dep_ptr->value_ref();
        }

        F& func;
    };

    dep_holder_t m_deps;
    F m_func;
};


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

    UREACT_WARN_UNUSED_RESULT op_t steal_op()
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



//==================================================================================================
// [[section]] Reactive signals and observers
//==================================================================================================
namespace detail
{

template <typename node_t>
class reactive_base
{
public:
    reactive_base() = default;

    reactive_base( std::shared_ptr<node_t>&& ptr ) noexcept
        : m_ptr( std::move( ptr ) )
    {}

    UREACT_WARN_UNUSED_RESULT bool is_valid() const
    {
        return m_ptr != nullptr;
    }

    UREACT_WARN_UNUSED_RESULT bool equals( const reactive_base& other ) const
    {
        return this->m_ptr == other.m_ptr;
    }

    UREACT_WARN_UNUSED_RESULT context& get_context() const
    {
        return m_ptr->get_context();
    }

protected:
    std::shared_ptr<node_t> m_ptr;

    template <typename node_t_>
    friend const std::shared_ptr<node_t_>& get_node_ptr( const reactive_base<node_t_>& node );
};


template <typename node_t>
UREACT_WARN_UNUSED_RESULT const std::shared_ptr<node_t>& get_node_ptr(
    const reactive_base<node_t>& node )
{
    return node.m_ptr;
}


template <typename S>
class signal_base : public reactive_base<signal_node<S>>
{
public:
    signal_base() = default;

    template <typename T>
    explicit signal_base( T&& t )
        : signal_base::reactive_base( std::forward<T>( t ) )
    {}

protected:
    UREACT_WARN_UNUSED_RESULT const S& get_value() const
    {
        return this->m_ptr->value_ref();
    }
};

} // namespace detail


/*! @brief Reactive variable that can propagate its changes to dependents and react to changes of
 * its dependencies. (Specialization for non-reference types.)
 *
 *  A signal is a reactive variable that can propagate its changes to dependents
 *  and react to changes of its dependencies.
 *
 *  Instances of this class act as a proxies to signal nodes. It takes shared
 *  ownership of the node, so while it exists, the node will not be destroyed.
 *  Copy, move and assignment semantics are similar to std::shared_ptr.
 *
 *  signals are created by constructor functions, i.e. make_function.
 */
template <typename S>
class signal : public detail::signal_base<S>
{
protected:
    using node_t = detail::signal_node<S>;

public:
    using value_t = S;

    /// Default constructor that needed for UREACT_REACTIVE_REF for some reason
    /// @todo investigate and remove it if possible
    signal() = default;

    /**
     * Construct signal from the given node.
     * @todo make it private and allow to call it only from make_value function
     */
    explicit signal( std::shared_ptr<node_t>&& node_ptr )
        : signal::signal_base( std::move( node_ptr ) )
    {}

    /// Return value of linked node
    UREACT_WARN_UNUSED_RESULT const S& get() const
    {
        return this->get_value();
    }

    /// Return value of linked node
    UREACT_WARN_UNUSED_RESULT const S& operator()() const
    {
        return this->get_value();
    }
};


/*! @brief Reactive variable that can propagate its changes to dependents and react to changes of
 * its dependencies. (Specialization for references.)
 *
 *  A signal is a reactive variable that can propagate its changes to dependents
 *  and react to changes of its dependencies.
 *
 *  Instances of this class act as a proxies to signal nodes. It takes shared
 *  ownership of the node, so while it exists, the node will not be destroyed.
 *  Copy, move and assignment semantics are similar to std::shared_ptr.
 *
 *  signals are created by constructor functions, i.e. make_function.
 */
template <typename S>
class signal<S&> : public detail::signal_base<std::reference_wrapper<S>>
{
protected:
    using node_t = detail::signal_node<std::reference_wrapper<S>>;

public:
    using value_t = S;

    /// Default constructor that needed for UREACT_REACTIVE_REF for some reason
    /// @todo investigate and remove it if possible
    signal() = default;

    /**
     * Construct signal from given node.
     * @todo make it private and allow to call it only from make_value function
     */
    explicit signal( std::shared_ptr<node_t>&& node_ptr )
        : signal::signal_base( std::move( node_ptr ) )
    {}

    /// Return value of linked node
    UREACT_WARN_UNUSED_RESULT const S& value() const
    {
        return this->get_value();
    }

    /// Return value of linked node
    UREACT_WARN_UNUSED_RESULT const S& operator()() const
    {
        return this->get_value();
    }
};

template <typename S>
class function : public signal<S>
{
protected:
    using node_t = typename signal<S>::node_t;

public:
    function() = default;

    explicit function( std::shared_ptr<node_t>&& node_ptr )
        : function::signal( std::move( node_ptr ) )
    {}
};

namespace detail
{

/*! @brief Source signals which values can be manually changed.
 *
 *  This class extends the immutable signal interface with functions that support
 *  imperative value input. In the dataflow graph, input signals are sources.
 *  As such, they don't have any predecessors.
 */
template <typename S>
class value_base : public signal<S>
{
private:
    UREACT_WARN_UNUSED_RESULT auto get_value_node() const
    {
        return static_cast<value_node<S>*>( this->m_ptr.get() );
    }

protected:
    /**
     * Construct value from value_node.
     */
    template <class node_t>
    explicit value_base( std::shared_ptr<node_t>&& node_ptr )
        : value_base::signal( std::move( node_ptr ) )
    {}

    template <typename T>
    void set_value( T&& new_value ) const
    {
        auto node_ptr = get_value_node();
        auto& graph_ref = node_ptr->get_graph();

        graph_ref.push_input( node_ptr,
            [node_ptr, &new_value] { node_ptr->set_value( std::forward<T>( new_value ) ); } );
    }

    template <typename F>
    void modify_value( const F& func ) const
    {
        auto node_ptr = get_value_node();
        auto& graph_ref = node_ptr->get_graph();

        graph_ref.push_input( node_ptr, [node_ptr, &func] { node_ptr->modify_value( func ); } );
    }
};

} // namespace detail


/*! @brief Source signals which values can be manually changed.
 * (Specialization for non-reference types.)
 *
 *  This class extends the immutable signal interface with functions that support
 *  imperative value input. In the dataflow graph, input signals are sources.
 *  As such, they don't have any predecessors.
 *
 *  value is created by constructor function make_value.
 */
template <typename S>
class value : public detail::value_base<S>
{
private:
    using node_t = ::ureact::detail::value_node<S>;

public:
    /**
     * Construct value from value_node.
     * @todo make it private and allow to call it only from make_value function
     */
    explicit value( std::shared_ptr<node_t>&& node_ptr )
        : value::value_base( std::move( node_ptr ) )
    {}

    /**
     * @brief Set new signal value
     *
     * Set the the signal value of the linked value signal node to a new_value.
     * If the old value equals the new value, the call has no effect.
     *
     * Furthermore, if set was called inside of a transaction function, it will
     * return after the changed value has been set and change propagation is delayed
     * until the transaction function returns.
     * Otherwise, propagation starts immediately and Set blocks until it's done.
     */
    void set( const S& new_value ) const
    {
        this->set_value( new_value );
    }

    /// @copydoc set
    void set( S&& new_value ) const
    {
        this->set_value( std::move( new_value ) );
    }

    /**
     * @brief Operator version of set()
     *
     * Semantically equivalent to set().
     */
    void operator<<=( const S& new_value ) const
    {
        this->set_value( new_value );
    }

    /**
     * @brief Operator version of set()
     *
     * Semantically equivalent to set().
     */
    void operator<<=( S&& new_value ) const
    {
        this->set_value( std::move( new_value ) );
    }

    /**
     * @brief Modify current signal value in-place
     */
    template <typename F>
    void modify( const F& func ) const
    {
        this->modify_value( func );
    }
};


/*! @brief Source signals which values can be manually changed.
 * (Specialization for references.)
 *
 *  This class extends the immutable signal interface with functions that support
 *  imperative value input. In the dataflow graph, input signals are sources.
 *  As such, they don't have any predecessors.
 *
 *  value is created by constructor function make_value.
 */
template <typename S>
class value<S&> : public detail::value_base<std::reference_wrapper<S>>
{
private:
    using node_t = detail::value_node<std::reference_wrapper<S>>;

public:
    using value_t = S;

    /**
     * Construct value from value_node.
     * @todo make it private and allow to call it only from make_value function
     */
    explicit value( std::shared_ptr<node_t>&& node_ptr )
        : value::value_base( std::move( node_ptr ) )
    {}

    /**
     * @brief Set new signal value
     *
     * Set the the signal value of the linked value signal node to a new_value.
     * If the old value equals the new value, the call has no effect.
     *
     * Furthermore, if set was called inside of a transaction function, it will
     * return after the changed value has been set and change propagation is delayed
     * until the transaction function returns.
     * Otherwise, propagation starts immediately and Set blocks until it's done.
     */
    void set( std::reference_wrapper<S> new_value ) const
    {
        this->set_value( new_value );
    }

    /**
     * @brief Operator version of set()
     *
     * Semantically equivalent to set().
     */
    void operator<<=( std::reference_wrapper<S> new_value ) const
    {
        this->set_value( new_value );
    }
};


/// Proxy class that wraps several nodes into a tuple.
template <typename... values_t>
class signal_pack
{
public:
    explicit signal_pack( const signal<values_t>&... deps )
        : data( std::tie( deps... ) )
    {}

    template <typename... cur_values_t, typename append_value_t>
    signal_pack(
        const signal_pack<cur_values_t...>& cur_args, const signal<append_value_t>& new_arg )
        : data( std::tuple_cat( cur_args.data, std::tie( new_arg ) ) )
    {}

    std::tuple<const signal<values_t>&...> data;
};


namespace detail
{

/**
 * This class exposes additional type information of the linked node, which enables
 * r-value based node merging at construction time.
 * The primary use case for this is to avoid unnecessary nodes when creating signal
 * expression from overloaded arithmetic operators.
 *
 * temp_function shouldn't be used as an l-value type, but instead implicitly
 * converted to signal.
 */
template <typename S, typename op_t>
class temp_function : public function<S>
{
private:
    using node_t = signal_op_node<S, op_t>;

public:
    /**
     * Construct temp_function from value_node.
     * @todo make it private and allow to call it only from make_value function
     */
    explicit temp_function( std::shared_ptr<node_t>&& ptr )
        : temp_function::function( std::move( ptr ) )
    {}

    /// Return internal operator, leaving node invalid
    UREACT_WARN_UNUSED_RESULT op_t steal_op()
    {
        auto* node_ptr = static_cast<node_t*>( this->m_ptr.get() );
        return node_ptr->steal_op();
    }
};


template <typename V, typename S = std::decay_t<V>, class = std::enable_if_t<!is_signal_v<S>>>
UREACT_WARN_UNUSED_RESULT auto make_value_impl( context& context, V&& v )
{
    return value<S>(
        std::make_shared<::ureact::detail::value_node<S>>( context, std::forward<V>( v ) ) );
}

template <typename S>
UREACT_WARN_UNUSED_RESULT auto make_value_impl( context& context, std::reference_wrapper<S> v )
{
    return value<S&>(
        std::make_shared<::ureact::detail::value_node<std::reference_wrapper<S>>>( context, v ) );
}

template <typename V,
    typename S = std::decay_t<V>,
    typename inner_t = typename S::value_t,
    class = std::enable_if_t<is_signal_v<S>>>
UREACT_WARN_UNUSED_RESULT auto make_value_impl( context& context, V&& v )
{
    return value<signal<inner_t>>( std::make_shared<::ureact::detail::value_node<signal<inner_t>>>(
        context, std::forward<V>( v ) ) );
}


template <typename S, typename op_t, typename... Args>
UREACT_WARN_UNUSED_RESULT auto make_temp_function( context& context, Args&&... args )
{
    return temp_function<S, op_t>(
        std::make_shared<signal_op_node<S, op_t>>( context, std::forward<Args>( args )... ) );
}

} // namespace detail


/*! @brief Shared pointer like object that holds a strong reference to the observed subject
 *
 *  An instance of this class provides a unique handle to an observer which can
 *  be used to detach it explicitly. It also holds a strong reference to
 *  the observed subject, so while it exists the subject and therefore
 *  the observer will not be destroyed.
 *
 *  If the handle is destroyed without calling detach(), the lifetime of
 *  the observer is tied to the subject.
 */
class observer
{
private:
    using subject_ptr_t = std::shared_ptr<detail::observable_node>;
    using node_t = ::ureact::detail::observer_node;

public:
    /// Default constructor
    observer() = default;

    /// Move constructor
    observer( observer&& other ) noexcept
        : m_node_ptr( other.m_node_ptr )
        , m_subject_ptr( std::move( other.m_subject_ptr ) )
    {
        other.m_node_ptr = nullptr;
        other.m_subject_ptr.reset();
    }

    /// Node constructor
    observer( node_t* node_ptr, subject_ptr_t subject_ptr )
        : m_node_ptr( node_ptr )
        , m_subject_ptr( std::move( subject_ptr ) )
    {}

    /// Move assignment
    observer& operator=( observer&& other ) noexcept
    {
        m_node_ptr = other.m_node_ptr;
        m_subject_ptr = std::move( other.m_subject_ptr );

        other.m_node_ptr = nullptr;
        other.m_subject_ptr.reset();

        return *this;
    }

    /// Deleted copy constructor and assignment
    observer( const observer& ) = delete;
    observer& operator=( const observer& ) = delete;

    ~observer() = default;

    /// Manually detaches the linked observer node from its subject
    void detach()
    {
        assert( is_valid() );
        m_subject_ptr->unregister_observer( m_node_ptr );
    }

    /// Tests if this instance is linked to a node
    UREACT_WARN_UNUSED_RESULT bool is_valid() const
    {
        return m_node_ptr != nullptr;
    }

private:
    /// Owned by subject
    node_t* m_node_ptr = nullptr;

    /// While the observer handle exists, the subject is not destroyed
    subject_ptr_t m_subject_ptr = nullptr;
};


/// Takes ownership of an observer and automatically detaches it on scope exit.
class scoped_observer
{
public:
    /// Move constructor
    scoped_observer( scoped_observer&& other ) noexcept
        : m_obs( std::move( other.m_obs ) )
    {}

    /// Constructs instance from observer
    scoped_observer( observer&& obs )
        : m_obs( std::move( obs ) )
    {}

    // Move assignment
    scoped_observer& operator=( scoped_observer&& other ) noexcept
    {
        m_obs = std::move( other.m_obs );
        return *this;
    }

    /// Deleted default ctor, copy ctor and assignment
    scoped_observer() = delete;
    scoped_observer( const scoped_observer& ) = delete;
    scoped_observer& operator=( const scoped_observer& ) = delete;

    /// Destructor
    ~scoped_observer()
    {
        m_obs.detach();
    }

    /// Tests if this instance is linked to a node
    UREACT_WARN_UNUSED_RESULT bool is_valid() const
    {
        return m_obs.is_valid();
    }

private:
    observer m_obs;
};



//==================================================================================================
// [[section]] Operator overloads for signals for simplified signal creation
//==================================================================================================
namespace detail
{

template <template <typename> class functor_op,
    typename signal_t,
    typename val_t = typename signal_t::value_t,
    class = std::enable_if_t<is_signal_v<signal_t>>,
    typename F = functor_op<val_t>,
    typename S = std::invoke_result_t<F, val_t>,
    typename op_t = function_op<S, F, signal_node_ptr_t<val_t>>>
auto unary_operator_impl( const signal_t& arg )
{
    return make_temp_function<S, op_t>( arg.get_context(), F(), get_node_ptr( arg ) );
}

template <template <typename> class functor_op,
    typename val_t,
    typename op_in_t,
    typename F = functor_op<val_t>,
    typename S = std::invoke_result_t<F, val_t>,
    typename op_t = function_op<S, F, op_in_t>>
auto unary_operator_impl( temp_function<val_t, op_in_t>&& arg )
{
    return make_temp_function<S, op_t>( arg.get_context(), F(), arg.steal_op() );
}

template <template <typename, typename> class functor_op,
    typename left_signal_t,
    typename right_signal_t,
    typename left_val_t = typename left_signal_t::value_t,
    typename right_val_t = typename right_signal_t::value_t,
    class = std::enable_if_t<is_signal_v<left_signal_t>>,
    class = std::enable_if_t<is_signal_v<right_signal_t>>,
    typename F = functor_op<left_val_t, right_val_t>,
    typename S = std::invoke_result_t<F, left_val_t, right_val_t>,
    typename op_t = detail::function_op<S,
        F,
        detail::signal_node_ptr_t<left_val_t>,
        detail::signal_node_ptr_t<right_val_t>>>
auto binary_operator_impl( const left_signal_t& lhs, const right_signal_t& rhs )
{
    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return make_temp_function<S, op_t>( context, F(), get_node_ptr( lhs ), get_node_ptr( rhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_signal_t,
    typename right_val_in_t,
    typename left_val_t = typename left_signal_t::value_t,
    typename right_val_t = std::decay_t<right_val_in_t>,
    class = std::enable_if_t<is_signal_v<left_signal_t>>,
    class = std::enable_if_t<!is_signal_v<right_val_t>>,
    typename F = bind_right<functor_op, left_val_t, right_val_t>,
    typename S = std::invoke_result_t<F, left_val_t>,
    typename op_t = detail::function_op<S, F, detail::signal_node_ptr_t<left_val_t>>>
auto binary_operator_impl( const left_signal_t& lhs, right_val_in_t&& rhs )
{
    context& context = lhs.get_context();

    return make_temp_function<S, op_t>(
        context, F( std::forward<right_val_in_t>( rhs ) ), get_node_ptr( lhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_val_in_t,
    typename right_signal_t,
    typename left_val_t = std::decay_t<left_val_in_t>,
    typename right_val_t = typename right_signal_t::value_t,
    class = std::enable_if_t<!is_signal_v<left_val_t>>,
    class = std::enable_if_t<is_signal_v<right_signal_t>>,
    typename F = bind_left<functor_op, left_val_t, right_val_t>,
    typename S = std::invoke_result_t<F, right_val_t>,
    typename op_t = detail::function_op<S, F, detail::signal_node_ptr_t<right_val_t>>>
auto binary_operator_impl( left_val_in_t&& lhs, const right_signal_t& rhs )
{
    context& context = rhs.get_context();

    return make_temp_function<S, op_t>(
        context, F( std::forward<left_val_in_t>( lhs ) ), get_node_ptr( rhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_val_t,
    typename left_op_t,
    typename right_val_t,
    typename right_op_t,
    typename F = functor_op<left_val_t, right_val_t>,
    typename S = std::invoke_result_t<F, left_val_t, right_val_t>,
    typename op_t = detail::function_op<S, F, left_op_t, right_op_t>>
auto binary_operator_impl( detail::temp_function<left_val_t, left_op_t>&& lhs,
    detail::temp_function<right_val_t, right_op_t>&& rhs )
{
    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return make_temp_function<S, op_t>( context, F(), lhs.steal_op(), rhs.steal_op() );
}

template <template <typename, typename> class functor_op,
    typename left_val_t,
    typename left_op_t,
    typename right_signal_t,
    typename right_val_t = typename right_signal_t::value_t,
    class = std::enable_if_t<is_signal_v<right_signal_t>>,
    typename F = functor_op<left_val_t, right_val_t>,
    typename S = std::invoke_result_t<F, left_val_t, right_val_t>,
    typename op_t = detail::function_op<S, F, left_op_t, detail::signal_node_ptr_t<right_val_t>>>
auto binary_operator_impl(
    detail::temp_function<left_val_t, left_op_t>&& lhs, const right_signal_t& rhs )
{
    context& context = rhs.get_context();

    return make_temp_function<S, op_t>( context, F(), lhs.steal_op(), get_node_ptr( rhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_signal_t,
    typename right_val_t,
    typename right_op_t,
    typename left_val_t = typename left_signal_t::value_t,
    class = std::enable_if_t<is_signal_v<left_signal_t>>,
    typename F = functor_op<left_val_t, right_val_t>,
    typename S = std::invoke_result_t<F, left_val_t, right_val_t>,
    typename op_t = detail::function_op<S, F, detail::signal_node_ptr_t<left_val_t>, right_op_t>>
auto binary_operator_impl(
    const left_signal_t& lhs, detail::temp_function<right_val_t, right_op_t>&& rhs )
{
    context& context = lhs.get_context();

    return make_temp_function<S, op_t>( context, F(), get_node_ptr( lhs ), rhs.steal_op() );
}

template <template <typename, typename> class functor_op,
    typename left_val_t,
    typename left_op_t,
    typename right_val_in_t,
    typename right_val_t = std::decay_t<right_val_in_t>,
    class = std::enable_if_t<!is_signal_v<right_val_t>>,
    typename F = bind_right<functor_op, left_val_t, right_val_t>,
    typename S = std::invoke_result_t<F, left_val_t>,
    typename op_t = detail::function_op<S, F, left_op_t>>
auto binary_operator_impl(
    detail::temp_function<left_val_t, left_op_t>&& lhs, right_val_in_t&& rhs )
{
    context& context = lhs.get_context();

    return make_temp_function<S, op_t>(
        context, F( std::forward<right_val_in_t>( rhs ) ), lhs.steal_op() );
}

template <template <typename, typename> class functor_op,
    typename left_val_in_t,
    typename right_val_t,
    typename right_op_t,
    typename left_val_t = std::decay_t<left_val_in_t>,
    class = std::enable_if_t<!is_signal_v<left_val_t>>,
    typename F = bind_left<functor_op, left_val_t, right_val_t>,
    typename S = std::invoke_result_t<F, right_val_t>,
    typename op_t = detail::function_op<S, F, right_op_t>>
auto binary_operator_impl(
    left_val_in_t&& lhs, detail::temp_function<right_val_t, right_op_t>&& rhs )
{
    context& context = rhs.get_context();

    return make_temp_function<S, op_t>(
        context, F( std::forward<left_val_in_t>( lhs ) ), rhs.steal_op() );
}

} // namespace detail

#if !defined( UREACT_DOC )

#    define UREACT_DECLARE_UNARY_OP_FUNCTOR( op, name )                                            \
        namespace detail                                                                           \
        {                                                                                          \
        namespace op_functors                                                                      \
        {                                                                                          \
        template <typename T>                                                                      \
        struct op_functor_##name                                                                   \
        {                                                                                          \
            UREACT_WARN_UNUSED_RESULT auto operator()( const T& v ) const                          \
            {                                                                                      \
                return op v;                                                                       \
            }                                                                                      \
        };                                                                                         \
        } /* namespace op_functors */                                                              \
        } /* namespace detail */


#    define UREACT_DECLARE_BINARY_OP_FUNCTOR( op, name )                                           \
        namespace detail                                                                           \
        {                                                                                          \
        namespace op_functors                                                                      \
        {                                                                                          \
        template <typename L, typename R>                                                          \
        struct op_functor_##name                                                                   \
        {                                                                                          \
            UREACT_WARN_UNUSED_RESULT auto operator()( const L& lhs, const R& rhs ) const          \
            {                                                                                      \
                return lhs op rhs;                                                                 \
            }                                                                                      \
        };                                                                                         \
        } /* namespace op_functors */                                                              \
        } /* namespace detail */


#    define UREACT_DECLARE_UNARY_OP( op, name )                                                    \
        template <typename arg_t,                                                                  \
            template <typename> class functor_op = detail::op_functors::op_functor_##name>         \
        UREACT_WARN_UNUSED_RESULT auto operator op( arg_t&& arg )                                  \
            ->decltype( detail::unary_operator_impl<functor_op>( std::forward<arg_t>( arg ) ) )    \
        {                                                                                          \
            return detail::unary_operator_impl<functor_op>( std::forward<arg_t&&>( arg ) );        \
        }


#    define UREACT_DECLARE_BINARY_OP( op, name )                                                   \
        template <typename lhs_t,                                                                  \
            typename rhs_t,                                                                        \
            template <typename, typename> class functor_op                                         \
            = detail::op_functors::op_functor_##name>                                              \
        UREACT_WARN_UNUSED_RESULT auto operator op( lhs_t&& lhs, rhs_t&& rhs )                     \
            ->decltype( detail::binary_operator_impl<functor_op>(                                  \
                std::forward<lhs_t&&>( lhs ), std::forward<rhs_t&&>( rhs ) ) )                     \
        {                                                                                          \
            return detail::binary_operator_impl<functor_op>(                                       \
                std::forward<lhs_t&&>( lhs ), std::forward<rhs_t&&>( rhs ) );                      \
        }


#    define UREACT_DECLARE_UNARY_OPERATOR( op, name )                                              \
        UREACT_DECLARE_UNARY_OP_FUNCTOR( op, name )                                                \
        UREACT_DECLARE_UNARY_OP( op, name )


#    define UREACT_DECLARE_BINARY_OPERATOR( op, name )                                             \
        UREACT_DECLARE_BINARY_OP_FUNCTOR( op, name )                                               \
        UREACT_DECLARE_BINARY_OP( op, name )


#    if defined( __clang__ ) && defined( __clang_minor__ )
#        pragma clang diagnostic push
#        pragma clang diagnostic ignored "-Wunknown-warning-option"
#        pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#    endif

UREACT_DECLARE_UNARY_OPERATOR( +, unary_plus )
UREACT_DECLARE_UNARY_OPERATOR( -, unary_minus )
UREACT_DECLARE_UNARY_OPERATOR( !, logical_negation )

UREACT_DECLARE_BINARY_OPERATOR( +, addition )
UREACT_DECLARE_BINARY_OPERATOR( -, subtraction )
UREACT_DECLARE_BINARY_OPERATOR( *, multiplication )
UREACT_DECLARE_BINARY_OPERATOR( /, division )
UREACT_DECLARE_BINARY_OPERATOR( %, modulo )

UREACT_DECLARE_BINARY_OPERATOR( ==, equal )
UREACT_DECLARE_BINARY_OPERATOR( !=, not_equal )
UREACT_DECLARE_BINARY_OPERATOR( <, less )
UREACT_DECLARE_BINARY_OPERATOR( <=, less_equal )
UREACT_DECLARE_BINARY_OPERATOR( >, greater )
UREACT_DECLARE_BINARY_OPERATOR( >=, greater_equal )

UREACT_DECLARE_BINARY_OPERATOR( &&, logical_and )
UREACT_DECLARE_BINARY_OPERATOR( ||, logical_or )

#    if defined( __clang__ ) && defined( __clang_minor__ )
#        pragma clang diagnostic pop
#    endif

#    undef UREACT_DECLARE_UNARY_OPERATOR
#    undef UREACT_DECLARE_UNARY_OP_FUNCTOR
#    undef UREACT_DECLARE_UNARY_OP
#    undef UREACT_DECLARE_BINARY_OPERATOR
#    undef UREACT_DECLARE_BINARY_OP_FUNCTOR
#    undef UREACT_DECLARE_BINARY_OP

#endif // !defined(UREACT_DOC)



//==================================================================================================
// [[section]] Free functions for fun and profit
//==================================================================================================

/// Factory function to create ureact::value in the given context.
template <typename V>
UREACT_WARN_UNUSED_RESULT auto make_value( context& context, V&& value )
{
    return detail::make_value_impl( context, std::forward<V>( value ) );
}


/// Utility function to create a signal_pack from given signals.
template <typename... values_t>
UREACT_WARN_UNUSED_RESULT auto with( const signal<values_t>&... deps )
{
    return signal_pack<values_t...>( deps... );
}


/// Comma operator overload to create signal pack from two signals.
template <typename left_val_t, typename right_val_t>
UREACT_WARN_UNUSED_RESULT auto operator,(
    const signal<left_val_t>& a, const signal<right_val_t>& b )
{
    return signal_pack<left_val_t, right_val_t>( a, b );
}

/// Comma operator overload to append node to existing signal pack.
template <typename... cur_values_t, typename append_value_t>
UREACT_WARN_UNUSED_RESULT auto operator,(
    const signal_pack<cur_values_t...>& cur, const signal<append_value_t>& append )
{
    return signal_pack<cur_values_t..., append_value_t>( cur, append );
}


/// Free function to connect a signal to a function and return the resulting signal.
template <typename value_t,
    typename in_f,
    typename F = std::decay_t<in_f>,
    typename S = std::invoke_result_t<F, value_t>,
    typename op_t
    = ::ureact::detail::function_op<S, F, ::ureact::detail::signal_node_ptr_t<value_t>>>
UREACT_WARN_UNUSED_RESULT auto make_function( const signal<value_t>& arg, in_f&& func )
{
    context& context = arg.get_context();

    return detail::make_temp_function<S, op_t>(
        context, std::forward<in_f>( func ), get_node_ptr( arg ) );
}

/// Free function to connect multiple signals to a function and return the resulting signal.
template <typename... values_t,
    typename in_f,
    typename F = std::decay_t<in_f>,
    typename S = std::invoke_result_t<F, values_t...>,
    typename op_t
    = ::ureact::detail::function_op<S, F, ::ureact::detail::signal_node_ptr_t<values_t>...>>
UREACT_WARN_UNUSED_RESULT auto make_function(
    const signal_pack<values_t...>& arg_pack, in_f&& func )
{
    context& context = std::get<0>( arg_pack.data ).get_context();

    auto node_builder = [&context, &func]( const signal<values_t>&... args ) {
        return detail::make_temp_function<S, op_t>(
            context, std::forward<in_f>( func ), get_node_ptr( args )... );
    };

    return std::apply( node_builder, arg_pack.data );
}


/// operator| overload to connect a signal to a function and return the resulting signal.
template <typename F, typename T, class = std::enable_if_t<is_signal_v<T>>>
UREACT_WARN_UNUSED_RESULT auto operator|( const T& arg, F&& func )
{
    return ::ureact::make_function( arg, std::forward<F>( func ) );
}

/// operator| overload to connect multiple signals to a function and return the resulting signal.
template <typename F, typename... values_t>
UREACT_WARN_UNUSED_RESULT auto operator|( const signal_pack<values_t...>& arg_pack, F&& func )
{
    return ::ureact::make_function( arg_pack, std::forward<F>( func ) );
}


template <typename inner_value_t>
UREACT_WARN_UNUSED_RESULT auto flatten( const signal<signal<inner_value_t>>& outer )
{
    context& context = outer.get_context();
    return function<inner_value_t>(
        std::make_shared<::ureact::detail::flatten_node<signal<inner_value_t>, inner_value_t>>(
            context, get_node_ptr( outer ), get_node_ptr( outer.get() ) ) );
}


namespace detail
{

template <typename in_f, typename S>
auto observe_impl( const signal<S>& subject, in_f&& func ) -> observer
{
    static_assert( std::is_invocable_v<in_f, S>,
        "Passed functor should be callable with S. See documentation for ureact::observe()" );

    using F = std::decay_t<in_f>;
    using R = std::invoke_result_t<in_f, S>;
    using wrapper_t = add_default_return_value_wrapper<F, observer_action, observer_action::next>;

    // If return value of passed function is void, add observer_action::next as
    // default return value.
    using node_t = std::conditional_t<std::is_same_v<void, R>,
        signal_observer_node<S, wrapper_t>,
        signal_observer_node<S, F>>;

    const auto& subject_ptr = get_node_ptr( subject );

    std::unique_ptr<observer_node> node_ptr(
        new node_t( subject.get_context(), subject_ptr, std::forward<in_f>( func ) ) );
    observer_node* raw_node_ptr = node_ptr.get();

    subject_ptr->register_observer( std::move( node_ptr ) );

    return observer( raw_node_ptr, subject_ptr );
}

}


/// When the signal value S of subject changes, func(s) is called.
/// The signature of func should be equivalent to:
/// TRet func(const S&)
/// TRet can be either observer_action or void.
/// By returning observer_action::stop_and_detach, the observer function can request
/// its own detachment. Returning observer_action::next keeps the observer attached.
/// Using a void return type is the same as always returning observer_action::next.
template <typename in_f, typename S>
auto observe( const signal<S>& subject, in_f&& func ) -> observer
{
    return ::ureact::detail::observe_impl( subject, std::forward<in_f>( func ) );
}

/// observe overload for temporary subject.
/// Caller must use result, otherwise observation isn't performed, that is not expected.
template <typename in_f, typename S>
UREACT_WARN_UNUSED_RESULT auto observe( signal<S>&& subject, in_f&& func ) -> observer
{
    return ::ureact::detail::observe_impl( subject, std::forward<in_f>( func ) );
}


#define UREACT_REACTIVE_REF( obj, name )                                                           \
    flatten( make_function(                                                                        \
        obj, []( const typename ::ureact::detail::type_identity_t<decltype( obj )>::value_t& r ) { \
            using T = decltype( r.name );                                                          \
            using S = ::ureact::detail::decay_input_t<T>;                                          \
            return static_cast<S>( r.name );                                                       \
        } ) )

#define UREACT_REACTIVE_PTR( obj, name )                                                           \
    flatten( make_function(                                                                        \
        obj, []( typename ::ureact::detail::type_identity_t<decltype( obj )>::value_t r ) {        \
            assert( r != nullptr );                                                                \
            using T = decltype( r->name );                                                         \
            using S = ::ureact::detail::decay_input_t<T>;                                          \
            return static_cast<S>( r->name );                                                      \
        } ) )



//==================================================================================================
// [[section]] Context class
//==================================================================================================

/*! @brief Core class that connects all reactive nodes together.
 *
 *  Each signal and node belongs to a single ureact context.
 *  Signals from different contexts can't interact with each other.
 */
class context : protected detail::context_internals
{
public:
    /// Perform several changes atomically
    template <typename F>
    void do_transaction( F&& func )
    {
        get_graph().do_transaction( std::forward<F>( func ) );
    }

    /// Factory function to create ureact::value in the current context.
    template <typename V>
    UREACT_WARN_UNUSED_RESULT auto make_value( V&& value )
    {
        return ureact::make_value( *this, std::forward<V>( value ) );
    }

    UREACT_WARN_UNUSED_RESULT bool operator==( const context& rsh ) const
    {
        return this == &rsh;
    }

    UREACT_WARN_UNUSED_RESULT bool operator!=( const context& rsh ) const
    {
        return this != &rsh;
    }

    /// Return internals. Not intended to use in user code.
    UREACT_WARN_UNUSED_RESULT friend detail::context_internals& _get_internals( context& ctx )
    {
        return ctx;
    }
};

UREACT_END_NAMESPACE

#endif // UREACT_UREACT_H_
