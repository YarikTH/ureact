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
#include <cstddef>
#include <cstdint>
#include <deque>
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
class var_signal;

template <typename E>
class events;

template <typename E>
class event_source;

template <typename... values_t>
class signal_pack;

template <typename E>
class event_range;

namespace detail
{

template <typename S>
class signal_node;

template <typename E>
class event_stream_node;

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

/// Return if type is events or its inheritor
template <typename T>
struct is_event : detail::is_base_of_template<events, T>
{};

template <typename T>
inline constexpr bool is_event_v = is_event<T>::value;


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

struct dont_move
{};

template <typename T, typename U>
using disable_if_same_t = std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::decay_t<U>>>;

template <typename T1, typename T2>
using is_same_decay = std::is_same<std::decay_t<T1>, std::decay_t<T2>>;

template <typename T1, typename T2>
inline constexpr bool is_same_decay_v = is_same_decay<T1, T2>::value;

} // namespace detail



//==================================================================================================
// [[section]] Ureact specific utilities
//==================================================================================================
namespace detail
{

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

template <typename L, typename R>
UREACT_WARN_UNUSED_RESULT bool equals( const events<L>& lhs, const events<R>& rhs )
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

template <typename node_type>
class node_vector
{
public:
    void add( node_type& node )
    {
        m_data.push_back( &node );
    }

    void remove( const node_type& node )
    {
        const auto it = detail::find( m_data.begin(), m_data.end(), &node );
        m_data.erase( it );
    }

    auto begin()
    {
        return m_data.begin();
    }

    auto end()
    {
        return m_data.end();
    }

private:
    std::vector<node_type*> m_data;
};


using turn_type = unsigned;


class reactive_node
{
public:
    int level{ 0 };
    int new_level{ 0 };
    bool queued{ false };

    node_vector<reactive_node> successors;

    virtual ~reactive_node() = default;

    virtual void tick( turn_type& turn ) = 0;
};


struct input_node_interface
{
    virtual ~input_node_interface() = default;

    virtual bool apply_input( turn_type& turn ) = 0;
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
        turn_type turn( next_turn_id() );

        // apply input node changes
        bool should_propagate = false;
        for( auto* p : m_changed_inputs )
        {
            if( p->apply_input( turn ) )
            {
                should_propagate = true;
            }
        }
        m_changed_inputs.clear();

        // propagate changes
        if( should_propagate )
        {
            propagate( turn );
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

    void propagate( turn_type& turn );

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

        [[nodiscard]] const std::vector<value_type>& next_values() const
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

    turn_type next_turn_id()
    {
        return m_next_turn_id++;
    }

    topological_queue m_scheduled_nodes;

    turn_type m_next_turn_id{ 0 };

    int m_transaction_level = 0;

    std::vector<input_node_interface*> m_changed_inputs;

    std::vector<observer_interface*> m_detached_observers;
};


UREACT_WARN_UNUSED_RESULT inline bool react_graph::topological_queue::fetch_next()
{
    // Throw away previous values
    m_next_data.clear();

    // Find min level of nodes in queue data
    auto minimal_level = std::numeric_limits<int>::max();
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
    parent.successors.add( node );

    if( node.level <= parent.level )
    {
        node.level = parent.level + 1;
    }
}

inline void react_graph::on_node_detach( reactive_node& node, reactive_node& parent )
{
    parent.successors.remove( node );
}

inline void react_graph::on_input_change( reactive_node& node )
{
    process_children( node );
}

inline void react_graph::on_node_pulse( reactive_node& node )
{
    process_children( node );
}

inline void react_graph::propagate( turn_type& turn )
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
            cur_node->tick( turn );
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

    // context_internals and context should be non-movable because
    // node_base contains reference to context and it will break if context lose its graph
    context_internals( context_internals&& ) noexcept = delete;
    context_internals& operator=( context_internals&& ) noexcept = delete;

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

    UREACT_WARN_UNUSED_RESULT bool operator==( const context& rsh ) const
    {
        return this == &rsh;
    }

    UREACT_WARN_UNUSED_RESULT bool operator!=( const context& rsh ) const
    {
        return this != &rsh;
    }

    /// Return internals. Not intended to use in user code.
    UREACT_WARN_UNUSED_RESULT friend context_internals& _get_internals( context& ctx )
    {
        return ctx;
    }
};


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

    node_base( const node_base& ) = delete;
    node_base& operator=( const node_base& ) = delete;

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


template <typename node_t, typename... deps_t>
struct attach_functor
{
    explicit attach_functor( node_t& node )
        : m_node( node )
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
        m_node.get_graph().on_node_attach( m_node, *dep_ptr );
    }

    node_t& m_node;
};


template <typename node_t, typename... deps_t>
struct detach_functor
{
    explicit detach_functor( node_t& node )
        : m_node( node )
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
        m_node.get_graph().on_node_detach( m_node, *dep_ptr );
    }

    node_t& m_node;
};


template <typename... deps_t>
class reactive_op_base
{
public:
    using dep_holder_t = std::tuple<deps_t...>;

    template <typename... deps_in_t>
    explicit reactive_op_base( dont_move, deps_in_t&&... deps )
        : m_deps( std::forward<deps_in_t>( deps )... )
    {}

    reactive_op_base( reactive_op_base&& ) noexcept = default;
    reactive_op_base& operator=( reactive_op_base&& ) noexcept = default;

    template <typename node_t>
    void attach( node_t& node ) const
    {
        std::apply( attach_functor<node_t, deps_t...>{ node }, m_deps );
    }

    template <typename node_t>
    void detach( node_t& node ) const
    {
        std::apply( detach_functor<node_t, deps_t...>{ node }, m_deps );
    }

    template <typename node_t, typename functor_t>
    void attach_rec( const functor_t& functor ) const
    {
        // Same memory layout, different func
        std::apply( reinterpret_cast<const attach_functor<node_t, deps_t...>&>( functor ), m_deps );
    }

    template <typename node_t, typename functor_t>
    void detach_rec( const functor_t& functor ) const
    {
        std::apply( reinterpret_cast<const detach_functor<node_t, deps_t...>&>( functor ), m_deps );
    }

protected:
    dep_holder_t m_deps;
};


template <typename node_t>
class reactive_base
{
public:
    reactive_base() = default;

    explicit reactive_base( std::shared_ptr<node_t>&& ptr )
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

    // LCOV_EXCL_START
    void tick( turn_type& ) override
    {
        assert( false && "Ticked var_node" );
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

    bool apply_input( turn_type& ) override
    {
        if( m_is_input_added )
        {
            m_is_input_added = false;

            if( !equals( this->m_value, m_new_value ) )
            {
                this->m_value = std::move( m_new_value );
                this->get_graph().on_input_change( *this );
                return true;
            }
            return false;
        }
        if( m_is_input_modified )
        {
            m_is_input_modified = false;

            this->get_graph().on_input_change( *this );
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
class function_op : public reactive_op_base<deps_t...>
{
public:
    template <typename f_in_t, typename... deps_in_t>
    explicit function_op( f_in_t&& func, deps_in_t&&... deps )
        : function_op::reactive_op_base( dont_move(), std::forward<deps_in_t>( deps )... )
        , m_func( std::forward<f_in_t>( func ) )
    {}

    function_op( function_op&& ) noexcept = default;
    function_op& operator=( function_op&& ) noexcept = default;

    UREACT_WARN_UNUSED_RESULT S evaluate()
    {
        return apply( eval_functor( m_func ), this->m_deps );
    }

private:
    struct eval_functor
    {
        explicit eval_functor( F& f )
            : m_func( f )
        {}

        template <typename... T>
        UREACT_WARN_UNUSED_RESULT S operator()( T&&... args )
        {
            return m_func( eval( args )... );
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

        F& m_func;
    };

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

    ~signal_op_node() override
    {
        if( !m_was_op_stolen )
        {
            m_op.detach( *this );
        }
    }

    void tick( turn_type& ) override
    {
        bool changed = false;

        {
            S new_value = m_op.evaluate();

            if( !equals( this->m_value, new_value ) )
            {
                this->m_value = std::move( new_value );
                changed = true;
            }
        }

        if( changed )
        {
            this->get_graph().on_node_pulse( *this );
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


template <typename S>
class signal_base : public reactive_base<signal_node<S>>
{
public:
    signal_base() = default;

    template <typename T, class = disable_if_same_t<T, signal_base>>
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
 *  signals are created by constructor functions, i.e. make_signal.
 */
template <typename S>
class signal : public detail::signal_base<S>
{
protected:
    using node_t = detail::signal_node<S>;

public:
    using value_t = S;

    /// Default constructor
    signal() = default;

    /**
     * Construct signal from the given node.
     * @todo make it private and allow to call it only from make_var function
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
 *  signals are created by constructor functions, i.e. make_signal.
 */
template <typename S>
class signal<S&> : public detail::signal_base<std::reference_wrapper<S>>
{
protected:
    using node_t = detail::signal_node<std::reference_wrapper<S>>;

public:
    using value_t = S;

    /// Default constructor
    signal() = default;

    /**
     * Construct signal from given node.
     * @todo make it private and allow to call it only from make_var function
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


namespace detail
{

/*! @brief Source signals which values can be manually changed.
 *
 *  This class extends the immutable signal interface with functions that support
 *  imperative value input. In the dataflow graph, input signals are sources.
 *  As such, they don't have any predecessors.
 */
template <typename S>
class var_signal_base : public signal<S>
{
private:
    UREACT_WARN_UNUSED_RESULT auto get_var_node() const
    {
        return static_cast<var_node<S>*>( this->m_ptr.get() );
    }

protected:
    /**
     * Construct value from var_node.
     */
    template <class node_t>
    explicit var_signal_base( std::shared_ptr<node_t>&& node_ptr )
        : var_signal_base::signal( std::move( node_ptr ) )
    {}

    template <typename T>
    void set_value( T&& new_value ) const
    {
        auto node_ptr = get_var_node();
        auto& graph_ref = node_ptr->get_graph();

        graph_ref.push_input( node_ptr,
            [node_ptr, &new_value] { node_ptr->set_value( std::forward<T>( new_value ) ); } );
    }

    template <typename F>
    void modify_value( const F& func ) const
    {
        auto node_ptr = get_var_node();
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
 *  var_signal is created by constructor function make_var.
 */
template <typename S>
class var_signal : public detail::var_signal_base<S>
{
private:
    using node_t = detail::var_node<S>;

public:
    /// Default ctor
    var_signal() = default;

    /**
     * Construct var_signal from var_node.
     * @todo make it private and allow to call it only from make_var function
     */
    explicit var_signal( std::shared_ptr<node_t>&& node_ptr )
        : var_signal::var_signal_base( std::move( node_ptr ) )
    {}

    /**
     * @brief Set new signal value
     *
     * Set the the signal value of the linked variable signal node to a new_value.
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
 *  var_signal is created by constructor function make_var.
 */
template <typename S>
class var_signal<S&> : public detail::var_signal_base<std::reference_wrapper<S>>
{
private:
    using node_t = detail::var_node<std::reference_wrapper<S>>;

public:
    using value_t = S;

    /// Default ctor
    var_signal() = default;

    /**
     * Construct var_signal from var_node.
     * @todo make it private and allow to call it only from make_var function
     */
    explicit var_signal( std::shared_ptr<node_t>&& node_ptr )
        : var_signal::var_signal_base( std::move( node_ptr ) )
    {}

    /**
     * @brief Set new signal value
     *
     * Set the the signal value of the linked variable signal node to a new_value.
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
 * temp_signal shouldn't be used as an l-value type, but instead implicitly
 * converted to signal.
 */
template <typename S, typename op_t>
class temp_signal : public signal<S>
{
private:
    using node_t = signal_op_node<S, op_t>;

public:
    /**
     * Construct temp_signal from var_node.
     * @todo make it private and allow to call it only from make_var function
     */
    explicit temp_signal( std::shared_ptr<node_t>&& ptr )
        : temp_signal::signal( std::move( ptr ) )
    {}

    /// Return internal operator, leaving node invalid
    UREACT_WARN_UNUSED_RESULT op_t steal_op()
    {
        auto* node_ptr = static_cast<node_t*>( this->m_ptr.get() );
        return node_ptr->steal_op();
    }
};


template <typename S>
UREACT_WARN_UNUSED_RESULT auto make_var_impl( context& context, std::reference_wrapper<S> v )
{
    return var_signal<S&>( std::make_shared<var_node<std::reference_wrapper<S>>>( context, v ) );
}

template <typename V, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto make_var_impl( context& context, V&& v )
{
    if constexpr( is_signal_v<S> || is_event_v<S> )
    {
        using inner_t = typename S::value_t;
        if constexpr( is_signal_v<S> )
        {
            return var_signal<signal<inner_t>>(
                std::make_shared<var_node<signal<inner_t>>>( context, std::forward<V>( v ) ) );
        }
        else if constexpr( is_event_v<S> )
        {
            return var_signal<events<inner_t>>(
                std::make_shared<var_node<events<inner_t>>>( context, std::forward<V>( v ) ) );
        }
    }
    else
    {
        return var_signal<S>( std::make_shared<var_node<S>>( context, std::forward<V>( v ) ) );
    }
}


template <typename S, typename op_t, typename... Args>
UREACT_WARN_UNUSED_RESULT auto make_temp_signal( context& context, Args&&... args )
{
    return temp_signal<S, op_t>(
        std::make_shared<signal_op_node<S, op_t>>( context, std::forward<Args>( args )... ) );
}

} // namespace detail


/// Factory function to create var signal in the given context.
template <typename V>
UREACT_WARN_UNUSED_RESULT auto make_var( context& context, V&& value )
{
    return make_var_impl( context, std::forward<V>( value ) );
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
    typename op_t = detail::function_op<S, F, detail::signal_node_ptr_t<value_t>>>
UREACT_WARN_UNUSED_RESULT auto make_signal( const signal<value_t>& arg, in_f&& func )
{
    context& context = arg.get_context();

    return detail::make_temp_signal<S, op_t>(
        context, std::forward<in_f>( func ), get_node_ptr( arg ) );
}

/// Free function to connect multiple signals to a function and return the resulting signal.
template <typename... values_t,
    typename in_f,
    typename F = std::decay_t<in_f>,
    typename S = std::invoke_result_t<F, values_t...>,
    typename op_t = detail::function_op<S, F, detail::signal_node_ptr_t<values_t>...>>
UREACT_WARN_UNUSED_RESULT auto make_signal( const signal_pack<values_t...>& arg_pack, in_f&& func )
{
    context& context = std::get<0>( arg_pack.data ).get_context();

    auto node_builder = [&context, &func]( const signal<values_t>&... args ) {
        return detail::make_temp_signal<S, op_t>(
            context, std::forward<in_f>( func ), get_node_ptr( args )... );
    };

    return std::apply( node_builder, arg_pack.data );
}


/// operator| overload to connect a signal to a function and return the resulting signal.
template <typename F, typename T, class = std::enable_if_t<is_signal_v<T>>>
UREACT_WARN_UNUSED_RESULT auto operator|( const T& arg, F&& func )
{
    return make_signal( arg, std::forward<F>( func ) );
}

/// operator| overload to connect multiple signals to a function and return the resulting signal.
template <typename F, typename... values_t>
UREACT_WARN_UNUSED_RESULT auto operator|( const signal_pack<values_t...>& arg_pack, F&& func )
{
    return make_signal( arg_pack, std::forward<F>( func ) );
}

//==================================================================================================
// [[section]] Operator overloads for signals for simplified signal creation
//==================================================================================================
namespace detail
{

// Full analog of std::binder1st that removed in c++17
// See https://en.cppreference.com/w/cpp/utility/functional/binder12
template <template <typename, typename> class functor_binary_op,
    typename first_argument_type,
    typename second_argument_type,
    typename Fn = functor_binary_op<first_argument_type, second_argument_type>>
class binder1st
{
public:
    template <typename T, class = disable_if_same_t<T, binder1st>>
    explicit binder1st( T&& first_argument )
        : m_first_argument( std::forward<T>( first_argument ) )
    {}

    template <typename T>
    UREACT_WARN_UNUSED_RESULT auto operator()( T&& second_argument ) const
    {
        return m_fn( m_first_argument, std::forward<T>( second_argument ) );
    }

private:
    Fn m_fn{};
    first_argument_type m_first_argument;
};


// Full analog of std::binder2nd that removed in c++17
// See https://en.cppreference.com/w/cpp/utility/functional/binder12
template <template <typename, typename> class functor_binary_op,
    typename first_argument_type,
    typename second_argument_type,
    typename Fn = functor_binary_op<first_argument_type, second_argument_type>>
class binder2nd
{
public:
    template <typename T, class = disable_if_same_t<T, binder2nd>>
    explicit binder2nd( T&& second_argument )
        : m_second_argument( std::forward<T>( second_argument ) )
    {}

    template <typename T>
    UREACT_WARN_UNUSED_RESULT auto operator()( T&& first_argument ) const
    {
        return m_fn( std::forward<T>( first_argument ), m_second_argument );
    }

private:
    Fn m_fn{};
    second_argument_type m_second_argument;
};

template <template <typename> class functor_op,
    typename signal_t,
    typename val_t = typename signal_t::value_t,
    class = std::enable_if_t<is_signal_v<signal_t>>,
    typename F = functor_op<val_t>,
    typename S = std::invoke_result_t<F, val_t>,
    typename op_t = function_op<S, F, signal_node_ptr_t<val_t>>>
auto unary_operator_impl( const signal_t& arg )
{
    return make_temp_signal<S, op_t>( arg.get_context(), F(), get_node_ptr( arg ) );
}

template <template <typename> class functor_op,
    typename val_t,
    typename op_in_t,
    typename F = functor_op<val_t>,
    typename S = std::invoke_result_t<F, val_t>,
    typename op_t = function_op<S, F, op_in_t>>
auto unary_operator_impl( temp_signal<val_t, op_in_t>&& arg )
{
    return make_temp_signal<S, op_t>( arg.get_context(), F(), arg.steal_op() );
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
    typename op_t
    = function_op<S, F, signal_node_ptr_t<left_val_t>, signal_node_ptr_t<right_val_t>>>
auto binary_operator_impl( const left_signal_t& lhs, const right_signal_t& rhs )
{
    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return make_temp_signal<S, op_t>( context, F(), get_node_ptr( lhs ), get_node_ptr( rhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_signal_t,
    typename right_val_in_t,
    typename left_val_t = typename left_signal_t::value_t,
    typename right_val_t = std::decay_t<right_val_in_t>,
    class = std::enable_if_t<is_signal_v<left_signal_t>>,
    class = std::enable_if_t<!is_signal_v<right_val_t>>>
auto binary_operator_impl( const left_signal_t& lhs, right_val_in_t&& rhs )
{
    using F = binder2nd<functor_op, left_val_t, right_val_t>;
    using S = std::invoke_result_t<F, left_val_t>;
    using op_t = function_op<S, F, signal_node_ptr_t<left_val_t>>;

    context& context = lhs.get_context();

    return make_temp_signal<S, op_t>(
        context, F( std::forward<right_val_in_t>( rhs ) ), get_node_ptr( lhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_val_in_t,
    typename right_signal_t,
    typename left_val_t = std::decay_t<left_val_in_t>,
    typename right_val_t = typename right_signal_t::value_t,
    class = std::enable_if_t<!is_signal_v<left_val_t>>,
    class = std::enable_if_t<is_signal_v<right_signal_t>>>
auto binary_operator_impl( left_val_in_t&& lhs, const right_signal_t& rhs )
{
    using F = binder1st<functor_op, left_val_t, right_val_t>;
    using S = std::invoke_result_t<F, right_val_t>;
    using op_t = function_op<S, F, signal_node_ptr_t<right_val_t>>;

    context& context = rhs.get_context();

    return make_temp_signal<S, op_t>(
        context, F( std::forward<left_val_in_t>( lhs ) ), get_node_ptr( rhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_val_t,
    typename left_op_t,
    typename right_val_t,
    typename right_op_t,
    typename F = functor_op<left_val_t, right_val_t>,
    typename S = std::invoke_result_t<F, left_val_t, right_val_t>,
    typename op_t = function_op<S, F, left_op_t, right_op_t>>
auto binary_operator_impl(
    temp_signal<left_val_t, left_op_t>&& lhs, temp_signal<right_val_t, right_op_t>&& rhs )
{
    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return make_temp_signal<S, op_t>( context, F(), lhs.steal_op(), rhs.steal_op() );
}

template <template <typename, typename> class functor_op,
    typename left_val_t,
    typename left_op_t,
    typename right_signal_t,
    typename right_val_t = typename right_signal_t::value_t,
    class = std::enable_if_t<is_signal_v<right_signal_t>>,
    typename F = functor_op<left_val_t, right_val_t>,
    typename S = std::invoke_result_t<F, left_val_t, right_val_t>,
    typename op_t = function_op<S, F, left_op_t, signal_node_ptr_t<right_val_t>>>
auto binary_operator_impl( temp_signal<left_val_t, left_op_t>&& lhs, const right_signal_t& rhs )
{
    context& context = rhs.get_context();

    return make_temp_signal<S, op_t>( context, F(), lhs.steal_op(), get_node_ptr( rhs ) );
}

template <template <typename, typename> class functor_op,
    typename left_signal_t,
    typename right_val_t,
    typename right_op_t,
    typename left_val_t = typename left_signal_t::value_t,
    class = std::enable_if_t<is_signal_v<left_signal_t>>,
    typename F = functor_op<left_val_t, right_val_t>,
    typename S = std::invoke_result_t<F, left_val_t, right_val_t>,
    typename op_t = function_op<S, F, signal_node_ptr_t<left_val_t>, right_op_t>>
auto binary_operator_impl( const left_signal_t& lhs, temp_signal<right_val_t, right_op_t>&& rhs )
{
    context& context = lhs.get_context();

    return make_temp_signal<S, op_t>( context, F(), get_node_ptr( lhs ), rhs.steal_op() );
}

template <template <typename, typename> class functor_op,
    typename left_val_t,
    typename left_op_t,
    typename right_val_in_t,
    typename right_val_t = std::decay_t<right_val_in_t>,
    class = std::enable_if_t<!is_signal_v<right_val_t>>>
auto binary_operator_impl( temp_signal<left_val_t, left_op_t>&& lhs, right_val_in_t&& rhs )
{
    using F = binder2nd<functor_op, left_val_t, right_val_t>;
    using S = std::invoke_result_t<F, left_val_t>;
    using op_t = function_op<S, F, left_op_t>;

    context& context = lhs.get_context();

    return make_temp_signal<S, op_t>(
        context, F( std::forward<right_val_in_t>( rhs ) ), lhs.steal_op() );
}

template <template <typename, typename> class functor_op,
    typename left_val_in_t,
    typename right_val_t,
    typename right_op_t,
    typename left_val_t = std::decay_t<left_val_in_t>,
    class = std::enable_if_t<!is_signal_v<left_val_t>>>
auto binary_operator_impl( left_val_in_t&& lhs, temp_signal<right_val_t, right_op_t>&& rhs )
{
    using F = binder1st<functor_op, left_val_t, right_val_t>;
    using S = std::invoke_result_t<F, right_val_t>;
    using op_t = function_op<S, F, right_op_t>;

    context& context = rhs.get_context();

    return make_temp_signal<S, op_t>(
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

// arithmetic operators

UREACT_DECLARE_BINARY_OPERATOR( +, addition )
UREACT_DECLARE_BINARY_OPERATOR( -, subtraction )
UREACT_DECLARE_BINARY_OPERATOR( *, multiplication )
UREACT_DECLARE_BINARY_OPERATOR( /, division )
UREACT_DECLARE_BINARY_OPERATOR( %, modulo )
UREACT_DECLARE_UNARY_OPERATOR( +, unary_plus )
UREACT_DECLARE_UNARY_OPERATOR( -, unary_minus )

// relational operators

UREACT_DECLARE_BINARY_OPERATOR( ==, equal )
UREACT_DECLARE_BINARY_OPERATOR( !=, not_equal )
UREACT_DECLARE_BINARY_OPERATOR( <, less )
UREACT_DECLARE_BINARY_OPERATOR( <=, less_equal )
UREACT_DECLARE_BINARY_OPERATOR( >, greater )
UREACT_DECLARE_BINARY_OPERATOR( >=, greater_equal )

// logical operators

UREACT_DECLARE_BINARY_OPERATOR( &&, logical_and )
UREACT_DECLARE_BINARY_OPERATOR( ||, logical_or )
UREACT_DECLARE_UNARY_OPERATOR( !, logical_negation )

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
// [[section]] Events
//==================================================================================================
enum class token
{
    value
};

namespace detail
{

template <typename E>
class event_stream_node : public observable_node
{
public:
    using data_t = std::vector<E>;

    explicit event_stream_node( context& context )
        : observable_node( context )
    {}

    void set_current_turn( const turn_type& turn )
    {
        if( m_cur_turn_id != turn )
        {
            m_cur_turn_id = turn;
            m_events.clear();
        }
    }

    void set_current_turn_force_update( const turn_type& turn )
    {
        m_cur_turn_id = turn;
        m_events.clear();
    }

    void set_current_turn_force_update_no_clear( const turn_type& turn )
    {
        m_cur_turn_id = turn;
    }

    data_t& events()
    {
        return m_events;
    }

protected:
    data_t m_events;

private:
    unsigned m_cur_turn_id{ std::numeric_limits<unsigned>::max() };
};

template <typename E>
using event_stream_node_ptr_t = std::shared_ptr<event_stream_node<E>>;

template <typename E>
class event_source_node
    : public event_stream_node<E>
    , public input_node_interface
{
public:
    explicit event_source_node( context& context )
        : event_source_node::event_stream_node( context )
    {}

    ~event_source_node() override = default;

    // LCOV_EXCL_START
    void tick( turn_type& ) override
    {
        assert( false && "Ticked event_source_node" );
    }
    // LCOV_EXCL_STOP

    template <typename V>
    void emit_value( V&& v )
    {
        // Clear input from previous turn
        if( m_changed_flag )
        {
            m_changed_flag = false;
            this->m_events.clear();
        }

        this->m_events.push_back( std::forward<V>( v ) );
    }

    bool apply_input( turn_type& turn ) override
    {
        if( this->m_events.size() > 0 && !m_changed_flag )
        {
            this->set_current_turn_force_update_no_clear( turn );
            m_changed_flag = true;
            this->get_graph().on_input_change( *this );
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    bool m_changed_flag = false;
};

template <typename E, typename op_t>
class event_op_node : public event_stream_node<E>
{
public:
    template <typename... args_t>
    explicit event_op_node( context& context, args_t&&... args )
        : event_op_node::event_stream_node( context )
        , m_op( std::forward<args_t>( args )... )
    {
        m_op.attach( *this );
    }

    ~event_op_node() override
    {
        if( !m_was_op_stolen )
        {
            m_op.detach( *this );
        }
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );

        m_op.collect( turn, event_collector( this->m_events ) );

        if( !this->m_events.empty() )
        {
            this->get_graph().on_node_pulse( *this );
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
    struct event_collector
    {
        using data_t = typename event_op_node::data_t;

        explicit event_collector( data_t& events )
            : m_events( events )
        {}

        void operator()( const E& e ) const
        {
            m_events.push_back( e );
        }

        data_t& m_events;
    };

    op_t m_op;
    bool m_was_op_stolen = false;
};


template <typename E>
class event_stream_base : public reactive_base<event_stream_node<E>>
{
public:
    event_stream_base() = default;
    event_stream_base( const event_stream_base& ) = default;
    event_stream_base& operator=( const event_stream_base& ) = default;

    template <typename T, class = disable_if_same_t<T, event_stream_base>>
    explicit event_stream_base( T&& t )
        : event_stream_base::reactive_base( std::forward<T>( t ) )
    {}

private:
    [[nodiscard]] auto get_event_source_node() const -> event_source_node<E>*
    {
        return static_cast<event_source_node<E>*>( this->m_ptr.get() );
    }

protected:
    template <typename T>
    void emit( T&& e ) const
    {
        auto node_ptr = get_event_source_node();
        auto& graph_ref = node_ptr->get_graph();

        graph_ref.push_input(
            node_ptr, [node_ptr, &e] { node_ptr->emit_value( std::forward<T>( e ) ); } );
    }
};

} // namespace detail

/// make_event_source
template <typename E = token>
auto make_event_source( context& context ) -> event_source<E>
{
    return event_source<E>( std::make_shared<detail::event_source_node<E>>( context ) );
}

/// temp_events
template <typename E, typename op_t>
class temp_events : public events<E>
{
protected:
    using node_t = detail::event_op_node<E, op_t>;

public:
    // Default ctor
    temp_events() = default;

    // Copy ctor
    temp_events( const temp_events& ) = default;

    // Move ctor
    temp_events( temp_events&& other ) noexcept
        : temp_events::events( std::move( other ) )
    {}

    // Node ctor
    explicit temp_events( std::shared_ptr<node_t>&& node_ptr )
        : temp_events::events( std::move( node_ptr ) )
    {}

    // Copy assignment
    temp_events& operator=( const temp_events& ) = default;

    // Move assignment
    temp_events& operator=( temp_events&& other ) noexcept
    {
        temp_events::event_stream_base::operator=( std::move( other ) );
        return *this;
    }

    op_t steal_op()
    {
        return std::move( reinterpret_cast<node_t*>( this->m_ptr.get() )->steal_op() );
    }
};

/// events
template <typename E = token>
class events : public detail::event_stream_base<E>
{
private:
    using node_t = detail::event_stream_node<E>;

public:
    using value_t = E;

    // Default ctor
    events() = default;

    // Copy ctor
    events( const events& ) = default;

    // Move ctor
    events( events&& other ) noexcept
        : events::event_stream_base( std::move( other ) )
    {}

    // Node ctor
    explicit events( std::shared_ptr<node_t>&& node_ptr )
        : events::event_stream_base( std::move( node_ptr ) )
    {}

    // Copy assignment
    events& operator=( const events& ) = default;

    // Move assignment
    events& operator=( events&& other ) noexcept
    {
        events::event_stream_base::operator=( std::move( other ) );
        return *this;
    }
};

// Specialize for references
template <typename E>
class events<E&> : public detail::event_stream_base<std::reference_wrapper<E>>
{
private:
    using node_t = detail::event_stream_node<std::reference_wrapper<E>>;

public:
    using value_t = E;

    // Default ctor
    events() = default;

    // Copy ctor
    events( const events& ) = default;

    // Move ctor
    events( events&& other ) noexcept
        : events::event_stream_base( std::move( other ) )
    {}

    // Node ctor
    explicit events( std::shared_ptr<node_t>&& node_ptr )
        : events::event_stream_base( std::move( node_ptr ) )
    {}

    // Copy assignment
    events& operator=( const events& ) = default;

    // Move assignment
    events& operator=( events&& other ) noexcept
    {
        events::event_stream_base::operator=( std::move( other ) );
        return *this;
    }
};

/// event_source
template <typename E = token>
class event_source : public events<E>
{
private:
    using node_t = detail::event_source_node<E>;

public:
    // Default ctor
    event_source() = default;

    // Copy ctor
    event_source( const event_source& ) = default;

    // Move ctor
    event_source( event_source&& other ) noexcept
        : event_source::events( std::move( other ) )
    {}

    // Node ctor
    explicit event_source( std::shared_ptr<node_t>&& node_ptr )
        : event_source::events( std::move( node_ptr ) )
    {}

    // Copy assignment
    event_source& operator=( const event_source& ) = default;

    // Move assignment
    event_source& operator=( event_source&& other ) noexcept
    {
        event_source::events::operator=( std::move( other ) );
        return *this;
    }

    // Explicit emit
    void emit( const E& e ) const
    {
        event_source::event_stream_base::emit( e );
    }

    void emit( E&& e ) const
    {
        event_source::event_stream_base::emit( std::move( e ) );
    }

    void emit() const
    {
        static_assert( std::is_same_v<E, token>, "Can't emit on non token stream." );
        event_source::event_stream_base::emit( token::value );
    }

    // Function object style
    void operator()( const E& e ) const
    {
        event_source::event_stream_base::emit( e );
    }

    void operator()( E&& e ) const
    {
        event_source::event_stream_base::emit( std::move( e ) );
    }

    void operator()() const
    {
        static_assert( std::is_same_v<E, token>, "Can't emit on non token stream." );
        event_source::event_stream_base::emit( token::value );
    }

    // Stream style
    const event_source& operator<<( const E& e ) const
    {
        event_source::event_stream_base::emit( e );
        return *this;
    }

    const event_source& operator<<( E&& e ) const
    {
        event_source::event_stream_base::emit( std::move( e ) );
        return *this;
    }
};

// Specialize for references
template <typename E>
class event_source<E&> : public events<std::reference_wrapper<E>>
{
private:
    using node_t = detail::event_source_node<std::reference_wrapper<E>>;

public:
    // Default ctor
    event_source() = default;

    // Copy ctor
    event_source( const event_source& ) = default;

    // Move ctor
    event_source( event_source&& other ) noexcept
        : event_source::events( std::move( other ) )
    {}

    // Node ctor
    explicit event_source( std::shared_ptr<node_t>&& node_ptr )
        : event_source::events( std::move( node_ptr ) )
    {}

    // Copy assignment
    event_source& operator=( const event_source& ) = default;

    // Move assignment
    event_source& operator=( event_source&& other ) noexcept
    {
        event_source::events::operator=( std::move( other ) );
        return *this;
    }

    // Explicit emit
    void emit( std::reference_wrapper<E> e ) const
    {
        event_source::event_stream_base::emit( e );
    }

    // Function object style
    void operator()( std::reference_wrapper<E> e ) const
    {
        event_source::event_stream_base::emit( e );
    }

    // Stream style
    const event_source& operator<<( std::reference_wrapper<E> e ) const
    {
        event_source::event_stream_base::emit( e );
        return *this;
    }
};

/// Iterators for event processing
template <typename E>
class event_range
{
public:
    using const_iterator = typename std::vector<E>::const_iterator;
    using size_type = typename std::vector<E>::size_type;

    [[nodiscard]] const_iterator begin() const
    {
        return m_data.begin();
    }

    [[nodiscard]] const_iterator end() const
    {
        return m_data.end();
    }

    [[nodiscard]] size_type size() const
    {
        return m_data.size();
    }

    [[nodiscard]] bool empty() const
    {
        return m_data.empty();
    }

    explicit event_range( const std::vector<E>& data )
        : m_data( data )
    {}

private:
    const std::vector<E>& m_data;
};

// Literally std::back_emplacer, but not depending on heavy <iterator> header
// and has additional << overload that matches more it this context
template <typename E>
class event_emitter
{
public:
    using container_type = std::vector<E>;

    explicit event_emitter( container_type& container )
        : m_container( container )
    {}

    event_emitter& operator*()
    {
        return *this;
    }

    event_emitter& operator++()
    {
        return *this;
    }

    event_emitter& operator++( int ) // NOLINT
    {
        return *this;
    }

    template <class T, class = detail::disable_if_same_t<T, event_emitter>>
    event_emitter& operator=( T&& value )
    {
        m_container.emplace_back( std::forward<T>( value ) );
        return *this;
    }

    template <class T>
    event_emitter& operator<<( T&& value )
    {
        m_container.emplace_back( std::forward<T>( value ) );
        return *this;
    }

private:
    container_type& m_container;
};

namespace detail
{

/// Operator | for function chaining
template <typename E, typename f_in_t>
UREACT_WARN_UNUSED_RESULT auto chain_algorithms_impl( const events<E>& source, f_in_t&& func )
{
    return std::forward<f_in_t>( func )( source );
}

/// Operator | for function chaining
template <typename E, typename op_in_t, typename f_in_t>
UREACT_WARN_UNUSED_RESULT auto chain_algorithms_impl(
    temp_events<E, op_in_t>&& source, f_in_t&& func )
{
    return std::forward<f_in_t>( func )( std::forward<temp_events<E, op_in_t>>( source ) );
}

template <typename E, typename... deps_t>
class event_merge_op : public reactive_op_base<deps_t...>
{
public:
    template <typename... deps_in_t>
    explicit event_merge_op( deps_in_t&&... deps )
        : event_merge_op::reactive_op_base( dont_move(), std::forward<deps_in_t>( deps )... )
    {}

    event_merge_op( event_merge_op&& other ) noexcept
        : event_merge_op::reactive_op_base( std::move( other ) )
    {}

    template <typename turn_t_, typename collector_t>
    void collect( const turn_t_& turn, const collector_t& collector ) const
    {
        apply( collect_functor<turn_t_, collector_t>( turn, collector ), this->m_deps );
    }

    template <typename turn_t_, typename collector_t, typename functor_t>
    void collect_rec( const functor_t& functor ) const
    {
        apply( reinterpret_cast<const collect_functor<turn_t_, collector_t>&>( functor ),
            this->m_deps );
    }

private:
    template <typename turn_t_, typename collector_t>
    struct collect_functor
    {
        collect_functor( const turn_t_& turn, const collector_t& collector )
            : m_turn( turn )
            , m_collector( collector )
        {}

        void operator()( const deps_t&... deps ) const
        {
            ( collect( deps ), ... );
        }

        template <typename T>
        void collect( const T& op ) const
        {
            op.template collect_rec<turn_t_, collector_t>( *this );
        }

        template <typename T>
        void collect( const std::shared_ptr<T>& dep_ptr ) const
        {
            dep_ptr->set_current_turn( m_turn );

            for( const auto& v : dep_ptr->events() )
            {
                m_collector( v );
            }
        }

        const turn_t_& m_turn;
        const collector_t& m_collector;
    };
};

template <typename E, typename filter_t, typename dep_t>
class event_filter_op : public reactive_op_base<dep_t>
{
public:
    template <typename filter_in_t, typename dep_in_t>
    event_filter_op( filter_in_t&& filter, dep_in_t&& dep )
        : event_filter_op::reactive_op_base{ dont_move(), std::forward<dep_in_t>( dep ) }
        , m_filter( std::forward<filter_in_t>( filter ) )
    {}

    event_filter_op( event_filter_op&& other ) noexcept
        : event_filter_op::reactive_op_base{ std::move( other ) }
        , m_filter( std::move( other.m_filter ) )
    {}

    template <typename turn_t_, typename collector_t>
    void collect( const turn_t_& turn, const collector_t& collector ) const
    {
        collect_impl(
            turn, filtered_event_collector<collector_t>{ m_filter, collector }, get_dep() );
    }

    template <typename turn_t_, typename collector_t, typename functor_t>
    void collect_rec( const functor_t& functor ) const
    {
        // Can't recycle functor because m_func needs replacing
        collect<turn_t_, collector_t>( functor.m_turn, functor.m_collector );
    }

private:
    const dep_t& get_dep() const
    {
        return std::get<0>( this->m_deps );
    }

    template <typename collector_t>
    struct filtered_event_collector
    {
        filtered_event_collector( const filter_t& filter, const collector_t& collector )
            : m_filter( filter )
            , m_collector( collector )
        {}

        void operator()( const E& e ) const
        {
            // Accepted?
            if( m_filter( e ) )
            {
                m_collector( e );
            }
        }

        const filter_t& m_filter;
        const collector_t& m_collector; // The wrapped collector
    };

    template <typename turn_t_, typename collector_t, typename T>
    static void collect_impl( const turn_t_& turn, const collector_t& collector, const T& op )
    {
        op.collect( turn, collector );
    }

    template <typename turn_t_, typename collector_t, typename T>
    static void collect_impl(
        const turn_t_& turn, const collector_t& collector, const std::shared_ptr<T>& dep_ptr )
    {
        dep_ptr->set_current_turn( turn );

        for( const auto& v : dep_ptr->events() )
        {
            collector( v );
        }
    }

    filter_t m_filter;
};

template <typename E, typename func_t, typename... dep_values_t>
class synced_event_filter_node : public event_stream_node<E>
{
public:
    template <typename F>
    synced_event_filter_node( context& context,
        const std::shared_ptr<event_stream_node<E>>& source,
        F&& filter,
        const std::shared_ptr<signal_node<dep_values_t>>&... deps )
        : synced_event_filter_node::event_stream_node( context )
        , m_source( source )
        , m_filter( std::forward<F>( filter ) )
        , m_deps( deps... )
    {
        this->get_graph().on_node_attach( *this, *source );
        ( this->get_graph().on_node_attach( *this, *deps ), ... );
    }

    ~synced_event_filter_node() override
    {
        this->get_graph().on_node_detach( *this, *m_source );

        apply(
            detach_functor<synced_event_filter_node, std::shared_ptr<signal_node<dep_values_t>>...>(
                *this ),
            m_deps );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );
        // Update of this node could be triggered from deps,
        // so make sure source doesn't contain events from last turn
        m_source->set_current_turn( turn );

        // Don't time if there is nothing to do
        if( !m_source->events().empty() )
        {
            for( const auto& e : m_source->events() )
            {
                if( apply(
                        [this, &e]( const std::shared_ptr<signal_node<dep_values_t>>&... args ) {
                            return m_filter( e, args->value_ref()... );
                        },
                        m_deps ) )
                {
                    this->m_events.push_back( e );
                }
            }
        }

        if( !this->m_events.empty() )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    using dep_holder_t = std::tuple<std::shared_ptr<signal_node<dep_values_t>>...>;

    std::shared_ptr<event_stream_node<E>> m_source;

    func_t m_filter;
    dep_holder_t m_deps;
};

// Todo: Refactor code duplication
template <typename E, typename func_t, typename dep_t>
class event_transform_op : public reactive_op_base<dep_t>
{
public:
    template <typename func_in_t, typename dep_in_t>
    event_transform_op( func_in_t&& func, dep_in_t&& dep )
        : event_transform_op::reactive_op_base( dont_move(), std::forward<dep_in_t>( dep ) )
        , m_func( std::forward<func_in_t>( func ) )
    {}

    event_transform_op( event_transform_op&& other ) noexcept
        : event_transform_op::reactive_op_base( std::move( other ) )
        , m_func( std::move( other.m_func ) )
    {}

    template <typename turn_t_, typename collector_t>
    void collect( const turn_t_& turn, const collector_t& collector ) const
    {
        collect_impl(
            turn, transform_event_collector<collector_t>( m_func, collector ), get_dep() );
    }

    template <typename turn_t_, typename collector_t, typename functor_t>
    void collect_rec( const functor_t& functor ) const
    {
        // Can't recycle functor because m_func needs replacing
        collect<turn_t_, collector_t>( functor.m_turn, functor.m_collector );
    }

private:
    const dep_t& get_dep() const
    {
        return std::get<0>( this->m_deps );
    }

    template <typename target_t>
    struct transform_event_collector
    {
        transform_event_collector( const func_t& func, const target_t& target )
            : m_func( func )
            , m_target( target )
        {}

        void operator()( const E& e ) const
        {
            m_target( m_func( e ) );
        }

        const func_t& m_func;
        const target_t& m_target;
    };

    template <typename turn_t_, typename collector_t, typename T>
    static void collect_impl( const turn_t_& turn, const collector_t& collector, const T& op )
    {
        op.collect( turn, collector );
    }

    template <typename turn_t_, typename collector_t, typename T>
    static void collect_impl(
        const turn_t_& turn, const collector_t& collector, const std::shared_ptr<T>& dep_ptr )
    {
        dep_ptr->set_current_turn( turn );

        for( const auto& v : dep_ptr->events() )
        {
            collector( v );
        }
    }

    func_t m_func;
};

template <typename in_t, typename out_t, typename func_t, typename... dep_values_t>
class synced_event_transform_node : public event_stream_node<out_t>
{
public:
    template <typename F>
    synced_event_transform_node( context& context,
        const std::shared_ptr<event_stream_node<in_t>>& source,
        F&& func,
        const std::shared_ptr<signal_node<dep_values_t>>&... deps )
        : synced_event_transform_node::event_stream_node( context )
        , m_source( source )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps... )
    {
        this->get_graph().on_node_attach( *this, *source );
        ( this->get_graph().on_node_attach( *this, *deps ), ... );
    }

    ~synced_event_transform_node() override
    {
        this->get_graph().on_node_detach( *this, *m_source );

        apply( detach_functor<synced_event_transform_node,
                   std::shared_ptr<signal_node<dep_values_t>>...>( *this ),
            m_deps );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );
        // Update of this node could be triggered from deps,
        // so make sure source doesn't contain events from last turn
        m_source->set_current_turn( turn );

        // Don't time if there is nothing to do
        if( !m_source->events().empty() )
        {
            for( const auto& e : m_source->events() )
            {
                this->m_events.push_back( apply(
                    [this, &e]( const std::shared_ptr<signal_node<dep_values_t>>&... args ) {
                        return m_func( e, args->value_ref()... );
                    },
                    m_deps ) );
            }
        }

        if( !this->m_events.empty() )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    using dep_holder_t = std::tuple<std::shared_ptr<signal_node<dep_values_t>>...>;

    std::shared_ptr<event_stream_node<in_t>> m_source;

    func_t m_func;
    dep_holder_t m_deps;
};

template <typename in_t, typename out_t, typename func_t>
class event_processing_node : public event_stream_node<out_t>
{
public:
    template <typename F>
    event_processing_node(
        context& context, const std::shared_ptr<event_stream_node<in_t>>& source, F&& func )
        : event_processing_node::event_stream_node( context )
        , m_source( source )
        , m_func( std::forward<F>( func ) )
    {
        this->get_graph().on_node_attach( *this, *source );
    }

    ~event_processing_node() override
    {
        this->get_graph().on_node_detach( *this, *m_source );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );

        m_func( event_range<in_t>( m_source->events() ), event_emitter( this->m_events ) );

        if( !this->m_events.empty() )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    std::shared_ptr<event_stream_node<in_t>> m_source;

    func_t m_func;
};

template <typename in_t, typename out_t, typename func_t, typename... dep_values_t>
class synced_event_processing_node : public event_stream_node<out_t>
{
public:
    template <typename F>
    synced_event_processing_node( context& context,
        const std::shared_ptr<event_stream_node<in_t>>& source,
        F&& func,
        const std::shared_ptr<signal_node<dep_values_t>>&... deps )
        : synced_event_processing_node::event_stream_node( context )
        , m_source( source )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps... )
    {
        this->get_graph().on_node_attach( *this, *source );
        ( this->get_graph().on_node_attach( *this, *deps ), ... );
    }

    ~synced_event_processing_node() override
    {
        this->get_graph().on_node_detach( *this, *m_source );

        apply( detach_functor<synced_event_processing_node,
                   std::shared_ptr<signal_node<dep_values_t>>...>( *this ),
            m_deps );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );
        // Update of this node could be triggered from deps,
        // so make sure source doesn't contain events from last turn
        m_source->set_current_turn( turn );

        // Don't time if there is nothing to do
        if( !m_source->events().empty() )
        {
            apply(
                [this]( const std::shared_ptr<signal_node<dep_values_t>>&... args ) {
                    m_func( event_range<in_t>( m_source->events() ),
                        event_emitter( this->m_events ),
                        args->value_ref()... );
                },
                m_deps );
        }

        if( !this->m_events.empty() )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    using dep_holder_t = std::tuple<std::shared_ptr<signal_node<dep_values_t>>...>;

    std::shared_ptr<event_stream_node<in_t>> m_source;

    func_t m_func;
    dep_holder_t m_deps;
};

template <typename... values_t>
class event_join_node : public event_stream_node<std::tuple<values_t...>>
{
public:
    explicit event_join_node(
        context& context, const std::shared_ptr<event_stream_node<values_t>>&... sources )
        : event_join_node::event_stream_node( context )
        , m_slots( sources... )
    {
        ( this->get_graph().on_node_attach( *this, *sources ), ... );
    }

    ~event_join_node() override
    {
        apply(
            [this]( slot<values_t>&... slots ) {
                ( this->get_graph().on_node_detach( *this, *slots.source ), ... );
            },
            m_slots );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );

        {
            // Move events into buffers
            apply( [&turn]( slot<values_t>&... slots ) { ( fetch_buffer( turn, slots ), ... ); },
                m_slots );

            while( true )
            {
                bool is_ready = true;

                // All slots ready?
                apply(
                    [&is_ready]( slot<values_t>&... slots ) {
                        // Todo: combine return values instead
                        ( check_slot( slots, is_ready ), ... );
                    },
                    m_slots );

                if( !is_ready )
                {
                    break;
                }

                // Pop values from buffers and emit tuple
                apply(
                    [this]( slot<values_t>&... slots ) {
                        this->m_events.emplace_back( slots.buffer.front()... );
                        ( slots.buffer.pop_front(), ... );
                    },
                    m_slots );
            }
        }

        if( !this->m_events.empty() )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    template <typename T>
    struct slot
    {
        explicit slot( const std::shared_ptr<event_stream_node<T>>& src )
            : source( src )
        {}

        std::shared_ptr<event_stream_node<T>> source;
        std::deque<T> buffer;
    };

    template <typename T>
    static void fetch_buffer( turn_type& turn, slot<T>& slot )
    {
        slot.source->set_current_turn( turn );

        slot.buffer.insert(
            slot.buffer.end(), slot.source->events().begin(), slot.source->events().end() );
    }

    template <typename T>
    static void check_slot( slot<T>& slot, bool& is_ready )
    {
        auto t = is_ready && !slot.buffer.empty();
        is_ready = t;
    }

    std::tuple<slot<values_t>...> m_slots;
};

} // namespace detail

/// Operator | for algorithms chaining
/// Usage: monitor( target ) | filter( is_even ) | tokenize();
template <typename S, typename f_in_t, class = std::enable_if_t<is_event_v<std::decay_t<S>>>>
UREACT_WARN_UNUSED_RESULT auto operator|( S&& source, f_in_t&& func )
{
    static_assert( std::is_invocable_v<f_in_t, decltype( source )>,
        "Function should be invocable with event type" );
    using result_t = std::decay_t<std::invoke_result_t<f_in_t, decltype( source )>>;
    static_assert( ureact::is_signal_v<result_t> || ureact::is_event_v<result_t>,
        "Function result should be signal or event" );
    return chain_algorithms_impl( std::forward<S>( source ), std::forward<f_in_t>( func ) );
}

/// merge
template <typename TArg1,
    typename... args_t,
    typename E = TArg1,
    typename op_t = detail::event_merge_op<E,
        detail::event_stream_node_ptr_t<TArg1>,
        detail::event_stream_node_ptr_t<args_t>...>>
auto merge( const events<TArg1>& arg1, const events<args_t>&... args ) -> temp_events<E, op_t>
{
    static_assert( sizeof...( args_t ) > 0, "merge: 2+ arguments are required." );

    context& context = arg1.get_context();
    return temp_events<E, op_t>( std::make_shared<detail::event_op_node<E, op_t>>(
        context, get_node_ptr( arg1 ), get_node_ptr( args )... ) );
}

/// filter
template <typename E,
    typename f_in_t,
    typename F = std::decay_t<f_in_t>,
    typename op_t = detail::event_filter_op<E, F, detail::event_stream_node_ptr_t<E>>>
auto filter( const events<E>& source, f_in_t&& func ) -> temp_events<E, op_t>
{
    context& context = source.get_context();
    return temp_events<E, op_t>( std::make_shared<detail::event_op_node<E, op_t>>(
        context, std::forward<f_in_t>( func ), get_node_ptr( source ) ) );
}

/// filter
template <typename E,
    typename op_in_t,
    typename f_in_t,
    typename F = std::decay_t<f_in_t>,
    typename op_out_t = detail::event_filter_op<E, F, op_in_t>>
auto filter( temp_events<E, op_in_t>&& source, f_in_t&& func ) -> temp_events<E, op_out_t>
{
    context& context = source.get_context();
    return temp_events<E, op_out_t>( std::make_shared<detail::event_op_node<E, op_out_t>>(
        context, std::forward<f_in_t>( func ), source.steal_op() ) );
}

/// filter - Synced
template <typename E, typename f_in_t, typename... dep_values_t>
auto filter( const events<E>& source, const signal_pack<dep_values_t...>& dep_pack, f_in_t&& func )
    -> events<E>
{
    using F = std::decay_t<f_in_t>;

    context& context = source.get_context();

    auto node_builder = [&context, &source, &func]( const signal<dep_values_t>&... deps ) {
        return events<E>(
            std::make_shared<detail::synced_event_filter_node<E, F, dep_values_t...>>( context,
                get_node_ptr( source ),
                std::forward<f_in_t>( func ),
                get_node_ptr( deps )... ) );
    };

    return std::apply( node_builder, dep_pack.data );
}

/// curried version of filter algorithm. Intended for chaining
template <typename f_in_t>
auto filter( f_in_t&& func )
{
    return [func = std::forward<f_in_t>( func )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( ureact::is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return filter( std::forward<arg_t>( source ), func );
    };
}

/// transform
template <typename in_t,
    typename f_in_t,
    typename F = std::decay_t<f_in_t>,
    typename out_t = std::invoke_result_t<F, in_t>,
    typename op_t = detail::event_transform_op<in_t, F, detail::event_stream_node_ptr_t<in_t>>>
auto transform( const events<in_t>& src, f_in_t&& func ) -> temp_events<out_t, op_t>
{
    context& context = src.get_context();
    return temp_events<out_t, op_t>( std::make_shared<detail::event_op_node<out_t, op_t>>(
        context, std::forward<f_in_t>( func ), get_node_ptr( src ) ) );
}

template <typename in_t,
    typename op_in_t,
    typename f_in_t,
    typename F = std::decay_t<f_in_t>,
    typename out_t = std::invoke_result_t<F, in_t>,
    typename op_out_t = detail::event_transform_op<in_t, F, op_in_t>>
auto transform( temp_events<in_t, op_in_t>&& src, f_in_t&& func ) -> temp_events<out_t, op_out_t>
{
    context& context = src.get_context();
    return temp_events<out_t, op_out_t>( std::make_shared<detail::event_op_node<out_t, op_out_t>>(
        context, std::forward<f_in_t>( func ), src.steal_op() ) );
}

/// transform - Synced
template <typename in_t,
    typename f_in_t,
    typename... dep_values_t,
    typename out_t = std::invoke_result_t<f_in_t, in_t, dep_values_t...>>
auto transform(
    const events<in_t>& source, const signal_pack<dep_values_t...>& dep_pack, f_in_t&& func )
    -> events<out_t>
{
    using F = std::decay_t<f_in_t>;

    context& context = source.get_context();

    auto node_builder = [&context, &source, &func]( const signal<dep_values_t>&... deps ) {
        return events<out_t>(
            std::make_shared<detail::synced_event_transform_node<in_t, out_t, F, dep_values_t...>>(
                context,
                get_node_ptr( source ),
                std::forward<f_in_t>( func ),
                get_node_ptr( deps )... ) );
    };

    return std::apply( node_builder, dep_pack.data );
}

/// process
template <typename out_t, typename in_t, typename f_in_t, typename F = std::decay_t<f_in_t>>
auto process( const events<in_t>& src, f_in_t&& func ) -> events<out_t>
{
    context& context = src.get_context();
    return events<out_t>( std::make_shared<detail::event_processing_node<in_t, out_t, F>>(
        context, get_node_ptr( src ), std::forward<f_in_t>( func ) ) );
}

/// process - Synced
template <typename out_t, typename in_t, typename f_in_t, typename... dep_values_t>
auto process(
    const events<in_t>& source, const signal_pack<dep_values_t...>& dep_pack, f_in_t&& func )
    -> events<out_t>
{
    using F = std::decay_t<f_in_t>;

    context& context = source.get_context();

    auto node_builder = [&context, &source, &func]( const signal<dep_values_t>&... deps ) {
        return events<out_t>(
            std::make_shared<detail::synced_event_processing_node<in_t, out_t, F, dep_values_t...>>(
                context,
                get_node_ptr( source ),
                std::forward<f_in_t>( func ),
                get_node_ptr( deps )... ) );
    };

    return std::apply( node_builder, dep_pack.data );
}

/// join
template <typename arg_t, typename... args_t>
auto join( const events<arg_t>& arg1, const events<args_t>&... args )
    -> events<std::tuple<arg_t, args_t...>>
{
    static_assert( sizeof...( args_t ) >= 1, "join: 2+ arguments are required." );

    context& context = arg1.get_context();
    return events<std::tuple<arg_t, args_t...>>(
        std::make_shared<detail::event_join_node<arg_t, args_t...>>(
            context, get_node_ptr( arg1 ), get_node_ptr( args )... ) );
}

template <typename events_t>
auto tokenize( events_t&& source )
{
    auto tokenizer = []( const auto& ) { return token::value; };
    return transform( source, tokenizer );
}

/// curried version of tokenize algorithm. Intended for chaining
inline auto tokenize()
{
    return []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( ureact::is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return tokenize( std::forward<arg_t>( source ) );
    };
}

//==================================================================================================
// [[section]] Observers
//==================================================================================================

/// Observer functions can return values of this type to control further processing.
enum class observer_action
{
    next,           ///< Need to continue observing
    stop_and_detach ///< Need to stop observing
};

namespace detail
{

/// Special wrapper to add specific return type to the void function
template <typename F, typename ret_t, ret_t return_value>
class add_default_return_value_wrapper
{
public:
    template <typename in_f,
        class = std::enable_if_t<!is_same_decay_v<in_f, add_default_return_value_wrapper>>>
    explicit add_default_return_value_wrapper( in_f&& func )
        : m_func( std::forward<in_f>( func ) )
    {}

    template <typename... args_t>
    UREACT_WARN_UNUSED_RESULT ret_t operator()( args_t&&... args )
    {
        m_func( std::forward<args_t>( args )... );
        return return_value;
    }

private:
    F m_func;
};

template <typename E, typename F, typename... args_t>
struct add_observer_range_wrapper
{
    add_observer_range_wrapper( const add_observer_range_wrapper& other ) = default;

    add_observer_range_wrapper( add_observer_range_wrapper&& other ) noexcept
        : m_func( std::move( other.m_func ) )
    {}

    template <typename f_in_t, class = disable_if_same_t<f_in_t, add_observer_range_wrapper>>
    explicit add_observer_range_wrapper( f_in_t&& func )
        : m_func( std::forward<f_in_t>( func ) )
    {}

    observer_action operator()( event_range<E> range, const args_t&... args )
    {
        for( const auto& e : range )
        {
            if( m_func( e, args... ) == observer_action::stop_and_detach )
            {
                return observer_action::stop_and_detach;
            }
        }

        return observer_action::next;
    }

    F m_func;
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

    void tick( turn_type& ) override
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

template <typename E, typename func_t>
class event_observer_node : public observer_node
{
public:
    template <typename F>
    event_observer_node(
        context& context, const std::shared_ptr<event_stream_node<E>>& subject, F&& func )
        : event_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<F>( func ) )
    {
        get_graph().on_node_attach( *this, *subject );
    }

    ~event_observer_node() override = default;

    void tick( turn_type& ) override
    {
        bool should_detach = false;

        if( auto p = m_subject.lock() )
        {
            should_detach
                = m_func( event_range<E>( p->events() ) ) == observer_action::stop_and_detach;
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
    std::weak_ptr<event_stream_node<E>> m_subject;

    func_t m_func;

    void detach_observer() override
    {
        if( auto p = m_subject.lock() )
        {
            get_graph().on_node_detach( *this, *p );
            m_subject.reset();
        }
    }
};

template <typename E, typename func_t, typename... dep_values_t>
class synced_observer_node : public observer_node
{
public:
    template <typename F>
    synced_observer_node( context& context,
        const std::shared_ptr<event_stream_node<E>>& subject,
        F&& func,
        const std::shared_ptr<signal_node<dep_values_t>>&... deps )
        : synced_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps... )
    {
        get_graph().on_node_attach( *this, *subject );
        ( get_graph().on_node_attach( *this, *deps ), ... );
    }

    ~synced_observer_node() override = default;

    void tick( turn_type& turn ) override
    {
        bool should_detach = false;

        if( auto p = m_subject.lock() )
        {
            // Update of this node could be triggered from deps,
            // so make sure source doesn't contain events from last turn
            p->set_current_turn( turn );

            {
                should_detach
                    = apply(
                          [this, &p]( const std::shared_ptr<signal_node<dep_values_t>>&... args ) {
                              return m_func( event_range<E>( p->events() ), args->value_ref()... );
                          },
                          m_deps )
                   == observer_action::stop_and_detach;
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
    using dep_holder_t = std::tuple<std::shared_ptr<signal_node<dep_values_t>>...>;

    std::weak_ptr<event_stream_node<E>> m_subject;

    func_t m_func;
    dep_holder_t m_deps;

    void detach_observer() override
    {
        if( auto p = m_subject.lock() )
        {
            get_graph().on_node_detach( *this, *p );

            apply(
                detach_functor<synced_observer_node, std::shared_ptr<signal_node<dep_values_t>>...>(
                    *this ),
                m_deps );

            m_subject.reset();
        }
    }
};

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
    using node_t = detail::observer_node;

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
    scoped_observer( observer&& obs ) // NOLINT no explicit by design
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

} // namespace detail

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
    return observe_impl( subject, std::forward<in_f>( func ) );
}

/// observe overload for temporary subject.
/// Caller must use result, otherwise observation isn't performed, that is not expected.
template <typename in_f, typename S>
UREACT_WARN_UNUSED_RESULT auto observe( signal<S>&& subject, in_f&& func ) -> observer
{
    return observe_impl( subject, std::forward<in_f>( func ) );
}

/// observe - events
template <typename f_in_t, typename E>
auto observe( const events<E>& subject, f_in_t&& func ) -> observer
{
    using F = std::decay_t<f_in_t>;

    using wrapper_t = std::conditional_t<std::is_invocable_r_v<observer_action, F, event_range<E>>,
        F,
        std::conditional_t<std::is_invocable_r_v<observer_action, F, E>,
            detail::add_observer_range_wrapper<E, F>,
            std::conditional_t<std::is_invocable_r_v<void, F, event_range<E>>,
                detail::add_default_return_value_wrapper<F, observer_action, observer_action::next>,
                std::conditional_t<std::is_invocable_r_v<void, F, E>,
                    detail::add_observer_range_wrapper<E,
                        detail::add_default_return_value_wrapper<F,
                            observer_action,
                            observer_action::next>>,
                    void>>>>;

    static_assert( !std::is_same_v<wrapper_t, void>,
        "observe: Passed function does not match any of the supported signatures." );

    using node_t = detail::event_observer_node<E, wrapper_t>;

    const auto& subject_ptr = get_node_ptr( subject );

    std::unique_ptr<detail::observer_node> node_ptr(
        new node_t( subject.get_context(), subject_ptr, std::forward<f_in_t>( func ) ) );
    detail::observer_node* raw_node_ptr = node_ptr.get();

    subject_ptr->register_observer( std::move( node_ptr ) );

    return observer( raw_node_ptr, subject_ptr );
}

/// observe - Synced
template <typename f_in_t, typename E, typename... dep_values_t>
auto observe(
    const events<E>& subject, const signal_pack<dep_values_t...>& dep_pack, f_in_t&& func )
    -> observer
{
    using F = std::decay_t<f_in_t>;

    using wrapper_t = std::conditional_t<
        std::is_invocable_r_v<observer_action, F, event_range<E>, dep_values_t...>,
        F,
        std::conditional_t<std::is_invocable_r_v<observer_action, F, E, dep_values_t...>,
            detail::add_observer_range_wrapper<E, F, dep_values_t...>,
            std::conditional_t<std::is_invocable_r_v<void, F, event_range<E>, dep_values_t...>,
                detail::add_default_return_value_wrapper<F, observer_action, observer_action::next>,
                std::conditional_t<std::is_invocable_r_v<void, F, E, dep_values_t...>,
                    detail::add_observer_range_wrapper<E,
                        detail::add_default_return_value_wrapper<F,
                            observer_action,
                            observer_action::next>,
                        dep_values_t...>,
                    void>>>>;

    static_assert( !std::is_same_v<wrapper_t, void>,
        "observe: Passed function does not match any of the supported signatures." );

    using node_t = detail::synced_observer_node<E, wrapper_t, dep_values_t...>;

    context& context = subject.get_context();

    auto node_builder = [&context, &subject, &func]( const signal<dep_values_t>&... deps ) {
        return new node_t( context,
            get_node_ptr( subject ),
            std::forward<f_in_t>( func ),
            get_node_ptr( deps )... );
    };

    const auto& subject_ptr = get_node_ptr( subject );

    std::unique_ptr<detail::observer_node> node_ptr( std::apply( node_builder, dep_pack.data ) );

    detail::observer_node* raw_node_ptr = node_ptr.get();

    subject_ptr->register_observer( std::move( node_ptr ) );

    return observer( raw_node_ptr, subject_ptr );
}


namespace detail
{

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
        this->get_graph().on_node_attach( *this, *m_outer );
        this->get_graph().on_node_attach( *this, *m_inner );
    }

    ~flatten_node() override
    {
        this->get_graph().on_node_detach( *this, *m_inner );
        this->get_graph().on_node_detach( *this, *m_outer );
    }

    void tick( turn_type& ) override
    {
        const auto& new_inner = get_node_ptr( m_outer->value_ref() );

        if( new_inner != m_inner )
        {
            // Topology has been changed
            auto old_inner = m_inner;
            m_inner = new_inner;

            this->get_graph().on_dynamic_node_detach( *this, *old_inner );
            this->get_graph().on_dynamic_node_attach( *this, *new_inner );

            return;
        }

        if( !equals( this->m_value, m_inner->value_ref() ) )
        {
            this->m_value = m_inner->value_ref();
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    std::shared_ptr<signal_node<outer_t>> m_outer;
    std::shared_ptr<signal_node<inner_t>> m_inner;
};


template <typename outer_t, typename inner_t>
class event_flatten_node : public event_stream_node<inner_t>
{
public:
    event_flatten_node( context& context,
        const std::shared_ptr<signal_node<outer_t>>& outer,
        const std::shared_ptr<event_stream_node<inner_t>>& inner )
        : event_flatten_node::event_stream_node( context )
        , m_outer( outer )
        , m_inner( inner )
    {
        this->get_graph().on_node_attach( *this, *m_outer );
        this->get_graph().on_node_attach( *this, *m_inner );
    }

    ~event_flatten_node() override
    {
        this->get_graph().on_node_detach( *this, *m_outer );
        this->get_graph().on_node_detach( *this, *m_inner );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );
        m_inner->set_current_turn( turn );

        auto new_inner = get_node_ptr( m_outer->value_ref() );

        if( new_inner != m_inner )
        {
            new_inner->set_current_turn( turn );

            // Topology has been changed
            auto m_old_inner = m_inner;
            m_inner = new_inner;

            this->get_graph().on_dynamic_node_detach( *this, *m_old_inner );
            this->get_graph().on_dynamic_node_attach( *this, *new_inner );

            return;
        }

        this->m_events.insert(
            this->m_events.end(), m_inner->events().begin(), m_inner->events().end() );

        if( this->m_events.size() > 0 )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    std::shared_ptr<signal_node<outer_t>> m_outer;
    std::shared_ptr<event_stream_node<inner_t>> m_inner;
};

template <typename T>
struct decay_input
{
    using type = T;
};

template <typename T>
struct decay_input<var_signal<T>>
{
    using type = signal<T>;
};

template <typename T>
using decay_input_t = typename decay_input<T>::type;

} // namespace detail

template <typename inner_value_t>
UREACT_WARN_UNUSED_RESULT auto flatten( const signal<signal<inner_value_t>>& outer )
{
    context& context = outer.get_context();
    return signal<inner_value_t>(
        std::make_shared<detail::flatten_node<signal<inner_value_t>, inner_value_t>>(
            context, get_node_ptr( outer ), get_node_ptr( outer.get() ) ) );
}

template <typename inner_value_t>
UREACT_WARN_UNUSED_RESULT auto flatten( const signal<events<inner_value_t>>& outer )
{
    context& context = outer.get_context();
    return events<inner_value_t>(
        std::make_shared<detail::event_flatten_node<events<inner_value_t>, inner_value_t>>(
            context, get_node_ptr( outer ), get_node_ptr( outer.get() ) ) );
}

template <typename S, typename R, typename decayed_r = detail::decay_input_t<R>>
auto reactive_ref( const ureact::signal<std::reference_wrapper<S>>& outer, R S::*attribute )
{
    return flatten( make_signal(
        outer, [attribute]( const S& s ) { return static_cast<decayed_r>( s.*attribute ); } ) );
}

template <typename S, typename R, typename decayed_r = detail::decay_input_t<R>>
auto reactive_ptr( const ureact::signal<S*>& outer, R S::*attribute )
{
    return flatten( make_signal(
        outer, [attribute]( const S* s ) { return static_cast<decayed_r>( s->*attribute ); } ) );
}

//==================================================================================================
// [[section]] Algorithms
//==================================================================================================

namespace detail
{

template <typename S>
class hold_node : public signal_node<S>
{
public:
    template <typename T>
    hold_node( context& context, T&& init, const std::shared_ptr<event_stream_node<S>>& events )
        : hold_node::signal_node( context, std::forward<T>( init ) )
        , m_events( events )
    {
        this->get_graph().on_node_attach( *this, *m_events );
    }

    ~hold_node() override
    {
        this->get_graph().on_node_detach( *this, *m_events );
    }

    void tick( turn_type& ) override
    {
        bool changed = false;

        if( !m_events->events().empty() )
        {
            const S& new_value = m_events->events().back();

            if( !equals( new_value, this->m_value ) )
            {
                changed = true;
                this->m_value = new_value;
            }
        }

        if( changed )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    const std::shared_ptr<event_stream_node<S>> m_events;
};

template <typename E>
class monitor_node : public event_stream_node<E>
{
public:
    monitor_node( context& context, const std::shared_ptr<signal_node<E>>& target )
        : monitor_node::event_stream_node( context )
        , m_target( target )
    {
        this->get_graph().on_node_attach( *this, *m_target );
    }

    ~monitor_node() override
    {
        this->get_graph().on_node_detach( *this, *m_target );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );

        this->m_events.push_back( m_target->value_ref() );

        if( !this->m_events.empty() )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    const std::shared_ptr<signal_node<E>> m_target;
};

template <typename E, typename S, typename F, typename... args_t>
struct add_iterate_range_wrapper
{
    add_iterate_range_wrapper( const add_iterate_range_wrapper& other ) = default;

    add_iterate_range_wrapper( add_iterate_range_wrapper&& other ) noexcept
        : m_func( std::move( other.m_func ) )
    {}

    template <typename f_in_t, class = disable_if_same_t<f_in_t, add_iterate_range_wrapper>>
    explicit add_iterate_range_wrapper( f_in_t&& func )
        : m_func( std::forward<f_in_t>( func ) )
    {}

    S operator()( event_range<E> range, S value, const args_t&... args )
    {
        for( const auto& e : range )
        {
            value = m_func( e, value, args... );
        }

        return value;
    }

    F m_func;
};

template <typename E, typename S, typename F, typename... args_t>
struct add_iterate_by_ref_range_wrapper
{
    add_iterate_by_ref_range_wrapper( const add_iterate_by_ref_range_wrapper& other ) = default;

    add_iterate_by_ref_range_wrapper( add_iterate_by_ref_range_wrapper&& other ) noexcept
        : m_func( std::move( other.m_func ) )
    {}

    template <typename f_in_t, class = disable_if_same_t<f_in_t, add_iterate_by_ref_range_wrapper>>
    explicit add_iterate_by_ref_range_wrapper( f_in_t&& func )
        : m_func( std::forward<f_in_t>( func ) )
    {}

    void operator()( event_range<E> range, S& value_ref, const args_t&... args )
    {
        for( const auto& e : range )
        {
            m_func( e, value_ref, args... );
        }
    }

    F m_func;
};

template <typename S, typename E, typename func_t>
class iterate_node : public signal_node<S>
{
public:
    template <typename T, typename F>
    iterate_node(
        context& context, T&& init, const std::shared_ptr<event_stream_node<E>>& events, F&& func )
        : iterate_node::signal_node( context, std::forward<T>( init ) )
        , m_events( events )
        , m_func( std::forward<F>( func ) )
    {
        this->get_graph().on_node_attach( *this, *events );
    }

    ~iterate_node() override
    {
        this->get_graph().on_node_detach( *this, *m_events );
    }

    void tick( turn_type& ) override
    {
        bool changed = false;

        {
            S new_value = m_func( event_range<E>( m_events->events() ), this->m_value );

            if( !equals( new_value, this->m_value ) )
            {
                this->m_value = std::move( new_value );
                changed = true;
            }
        }

        if( changed )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    std::shared_ptr<event_stream_node<E>> m_events;

    func_t m_func;
};

template <typename S, typename E, typename func_t>
class iterate_by_ref_node : public signal_node<S>
{
public:
    template <typename T, typename F>
    iterate_by_ref_node(
        context& context, T&& init, const std::shared_ptr<event_stream_node<E>>& events, F&& func )
        : iterate_by_ref_node::signal_node( context, std::forward<T>( init ) )
        , m_func( std::forward<F>( func ) )
        , m_events( events )
    {
        this->get_graph().on_node_attach( *this, *events );
    }

    ~iterate_by_ref_node() override
    {
        this->get_graph().on_node_detach( *this, *m_events );
    }

    void tick( turn_type& ) override
    {
        m_func( event_range<E>( m_events->events() ), this->m_value );

        // Always assume change
        this->get_graph().on_node_pulse( *this );
    }

protected:
    func_t m_func;

    std::shared_ptr<event_stream_node<E>> m_events;
};

template <typename S, typename E, typename func_t, typename... dep_values_t>
class synced_iterate_node : public signal_node<S>
{
public:
    template <typename T, typename F>
    synced_iterate_node( context& context,
        T&& init,
        const std::shared_ptr<event_stream_node<E>>& events,
        F&& func,
        const std::shared_ptr<signal_node<dep_values_t>>&... deps )
        : synced_iterate_node::signal_node( context, std::forward<T>( init ) )
        , m_events( events )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps... )
    {
        this->get_graph().on_node_attach( *this, *events );
        ( this->get_graph().on_node_attach( *this, *deps ), ... );
    }

    ~synced_iterate_node() override
    {
        this->get_graph().on_node_detach( *this, *m_events );

        apply( detach_functor<synced_iterate_node, std::shared_ptr<signal_node<dep_values_t>>...>(
                   *this ),
            m_deps );
    }

    void tick( turn_type& turn ) override
    {
        m_events->set_current_turn( turn );

        bool changed = false;

        if( !m_events->events().empty() )
        {
            S new_value = apply(
                [this]( const std::shared_ptr<signal_node<dep_values_t>>&... args ) {
                    return m_func(
                        event_range<E>( m_events->events() ), this->m_value, args->value_ref()... );
                },
                m_deps );

            if( !equals( new_value, this->m_value ) )
            {
                changed = true;
                this->m_value = std::move( new_value );
            }
        }

        if( changed )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    using dep_holder_t = std::tuple<std::shared_ptr<signal_node<dep_values_t>>...>;

    std::shared_ptr<event_stream_node<E>> m_events;

    func_t m_func;
    dep_holder_t m_deps;
};

template <typename S, typename E, typename func_t, typename... dep_values_t>
class synced_iterate_by_ref_node : public signal_node<S>
{
public:
    template <typename T, typename F>
    synced_iterate_by_ref_node( context& context,
        T&& init,
        const std::shared_ptr<event_stream_node<E>>& events,
        F&& func,
        const std::shared_ptr<signal_node<dep_values_t>>&... deps )
        : synced_iterate_by_ref_node::signal_node( context, std::forward<T>( init ) )
        , m_events( events )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps... )
    {
        this->get_graph().on_node_attach( *this, *events );
        ( this->get_graph().on_node_attach( *this, *deps ), ... );
    }

    ~synced_iterate_by_ref_node() override
    {
        this->get_graph().on_node_detach( *this, *m_events );

        apply( detach_functor<synced_iterate_by_ref_node,
                   std::shared_ptr<signal_node<dep_values_t>>...>( *this ),
            m_deps );
    }

    void tick( turn_type& turn ) override
    {
        m_events->set_current_turn( turn );

        bool changed = false;

        if( !m_events->events().empty() )
        {
            apply(
                [this]( const std::shared_ptr<signal_node<dep_values_t>>&... args ) {
                    m_func(
                        event_range<E>( m_events->events() ), this->m_value, args->value_ref()... );
                },
                m_deps );

            changed = true;
        }

        if( changed )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

private:
    using dep_holder_t = std::tuple<std::shared_ptr<signal_node<dep_values_t>>...>;

    std::shared_ptr<event_stream_node<E>> m_events;

    func_t m_func;
    dep_holder_t m_deps;
};

// Base class for snapshot and pulse algorithm
// do something with target signal value on trigger event fires
template <template <typename> class base, typename S, typename E>
class triggered_node : public base<S>
{
public:
    template <typename... args_t>
    triggered_node( context& context,
        const std::shared_ptr<signal_node<S>>& target,
        const std::shared_ptr<event_stream_node<E>>& trigger,
        args_t&&... args )
        : base<S>( context, std::forward<args_t>( args )... )
        , m_target( target )
        , m_trigger( trigger )
    {
        this->get_graph().on_node_attach( *this, *m_target );
        this->get_graph().on_node_attach( *this, *m_trigger );
    }

    ~triggered_node() override
    {
        this->get_graph().on_node_detach( *this, *m_target );
        this->get_graph().on_node_detach( *this, *m_trigger );
    }

    void tick( turn_type& turn ) final
    {
        before_tick( turn );

        m_trigger->set_current_turn( turn );

        const size_t count = m_trigger->events().size();
        if( count > 0 )
        {
            const S& new_value = m_target->value_ref();
            if( on_trigger_fires( count, new_value ) )
            {
                this->get_graph().on_node_pulse( *this );
            }
        }
    }

    /// some turn related actions at the begin of the spin
    virtual void before_tick( turn_type& turn )
    {}

    /// specific actions performed when trigger fires at least once
    virtual bool on_trigger_fires( size_t count, const S& target_value ) = 0;

private:
    const std::shared_ptr<signal_node<S>> m_target;
    const std::shared_ptr<event_stream_node<E>> m_trigger;
};

template <typename S, typename E>
class snapshot_node : public triggered_node<signal_node, S, E>
{
public:
    snapshot_node( context& context,
        const std::shared_ptr<signal_node<S>>& target,
        const std::shared_ptr<event_stream_node<E>>& trigger )
        : snapshot_node::triggered_node( context, target, trigger, target->value_ref() )
    {}

    bool on_trigger_fires( size_t, const S& target_value ) final
    {
        if( equals( target_value, this->m_value ) )
        {
            return false;
        }
        else
        {
            this->m_value = target_value;
            return true;
        }
    }
};

template <typename S, typename E>
class pulse_node : public triggered_node<event_stream_node, S, E>
{
public:
    pulse_node( context& context,
        const std::shared_ptr<signal_node<S>>& target,
        const std::shared_ptr<event_stream_node<E>>& trigger )
        : pulse_node::triggered_node( context, target, trigger )
    {}

    void before_tick( turn_type& turn ) final
    {
        this->set_current_turn_force_update( turn );
    }

    bool on_trigger_fires( size_t count, const S& target_value ) final
    {
        this->m_events.reserve( this->m_events.size() + count );

        for( size_t i = 0, ie = count; i < ie; ++i )
        {
            this->m_events.push_back( target_value );
        }

        return true;
    }
};

} // namespace detail

/// Holds most recent event in a signal
template <typename V, typename T = std::decay_t<V>>
auto hold( const events<T>& events, V&& init ) -> signal<T>
{
    context& context = events.get_context();
    return signal<T>( std::make_shared<detail::hold_node<T>>(
        context, std::forward<V>( init ), get_node_ptr( events ) ) );
}

/// Emits value changes of signal as events
template <typename S>
auto monitor( const signal<S>& target ) -> events<S>
{
    context& context = target.get_context();
    return events<S>(
        std::make_shared<detail::monitor_node<S>>( context, get_node_ptr( target ) ) );
}

/// Folds values from event stream into a signal
/// iterate - Iteratively combines signal value with values from event stream (aka Fold)
template <typename E, typename V, typename f_in_t, typename S = std::decay_t<V>>
auto iterate( const events<E>& events, V&& init, f_in_t&& func ) -> signal<S>
{
    using F = std::decay_t<f_in_t>;

    using node_t = std::conditional_t<std::is_invocable_r_v<S, F, event_range<E>, S>,
        detail::iterate_node<S, E, F>,
        std::conditional_t<std::is_invocable_r_v<S, F, E, S>,
            detail::iterate_node<S, E, detail::add_iterate_range_wrapper<E, S, F>>,
            std::conditional_t<std::is_invocable_r_v<void, F, event_range<E>, S&>,
                detail::iterate_by_ref_node<S, E, F>,
                std::conditional_t<std::is_invocable_r_v<void, F, E, S&>,
                    detail::iterate_by_ref_node<S,
                        E,
                        detail::add_iterate_by_ref_range_wrapper<E, S, F>>,
                    void>>>>;

    static_assert( !std::is_same_v<node_t, void>,
        "iterate: Passed function does not match any of the supported signatures." );

    context& context = events.get_context();
    return signal<S>( std::make_shared<node_t>(
        context, std::forward<V>( init ), get_node_ptr( events ), std::forward<f_in_t>( func ) ) );
}

/// iterate - Synced
template <typename E,
    typename V,
    typename f_in_t,
    typename... dep_values_t,
    typename S = std::decay_t<V>>
auto iterate(
    const events<E>& events, V&& init, const signal_pack<dep_values_t...>& dep_pack, f_in_t&& func )
    -> signal<S>
{
    using F = std::decay_t<f_in_t>;

    using node_t = std::conditional_t<
        std::is_invocable_r_v<S, F, event_range<E>, S, dep_values_t...>,
        detail::synced_iterate_node<S, E, F, dep_values_t...>,
        std::conditional_t<std::is_invocable_r_v<S, F, E, S, dep_values_t...>,
            detail::synced_iterate_node<S,
                E,
                detail::add_iterate_range_wrapper<E, S, F, dep_values_t...>,
                dep_values_t...>,
            std::conditional_t<std::is_invocable_r_v<void, F, event_range<E>, S&, dep_values_t...>,
                detail::synced_iterate_by_ref_node<S, E, F, dep_values_t...>,
                std::conditional_t<std::is_invocable_r_v<void, F, E, S&, dep_values_t...>,
                    detail::synced_iterate_by_ref_node<S,
                        E,
                        detail::add_iterate_by_ref_range_wrapper<E, S, F, dep_values_t...>,
                        dep_values_t...>,
                    void>>>>;

    static_assert( !std::is_same_v<node_t, void>,
        "iterate: Passed function does not match any of the supported signatures." );

    context& context = events.get_context();

    auto node_builder = [&context, &events, &init, &func]( const signal<dep_values_t>&... deps ) {
        return signal<S>( std::make_shared<node_t>( context,
            std::forward<V>( init ),
            get_node_ptr( events ),
            std::forward<f_in_t>( func ),
            get_node_ptr( deps )... ) );
    };

    return std::apply( node_builder, dep_pack.data );
}

/// snapshot - Sets signal value to value of other signal when event is received
template <typename S, typename E>
auto snapshot( const events<E>& trigger, const signal<S>& target ) -> signal<S>
{
    context& context = trigger.get_context();
    return signal<S>( std::make_shared<detail::snapshot_node<S, E>>(
        context, get_node_ptr( target ), get_node_ptr( trigger ) ) );
}

/// pulse - Emits value of target signal when event is received
template <typename S, typename E>
auto pulse( const events<E>& trigger, const signal<S>& target ) -> events<S>
{
    context& context = trigger.get_context();
    return events<S>( std::make_shared<detail::pulse_node<S, E>>(
        context, get_node_ptr( target ), get_node_ptr( trigger ) ) );
}

/// changed - Emits token when target signal was changed
template <typename S>
auto changed( const signal<S>& target ) -> events<token>
{
    return monitor( target ) | tokenize();
}

/// changed_to - Emits token when target signal was changed to value
template <typename V, typename S = std::decay_t<V>>
auto changed_to( const signal<S>& target, V&& value ) -> events<token>
{
    return monitor( target ) | filter( [=]( const S& v ) { return v == value; } ) | tokenize();
}

UREACT_END_NAMESPACE

#endif // UREACT_UREACT_H_
