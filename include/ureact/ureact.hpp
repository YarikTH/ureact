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

#if( UREACT_HAS_CPP17_ATTRIBUTE( nodiscard ) >= 201907L )
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

#define UREACT_MAKE_NONCOPYABLE( ClassName )                                                       \
    ClassName( const ClassName& ) = delete;                                                        \
    ClassName& operator=( const ClassName& ) = delete

#define UREACT_MAKE_NONMOVABLE( ClassName )                                                        \
    ClassName( ClassName&& ) noexcept = delete;                                                    \
    ClassName& operator=( ClassName&& ) noexcept = delete

#define UREACT_MAKE_COPYABLE( ClassName )                                                          \
    ClassName( const ClassName& ) = default;                                                       \
    ClassName& operator=( const ClassName& ) = default

#define UREACT_MAKE_MOVABLE( ClassName )                                                           \
    ClassName( ClassName&& ) noexcept = default;                                                   \
    ClassName& operator=( ClassName&& ) noexcept = default

#define UREACT_MAKE_MOVABLE_ONLY( ClassName )                                                      \
    UREACT_MAKE_NONCOPYABLE( ClassName );                                                          \
    UREACT_MAKE_MOVABLE( ClassName )

// Define function body using "Please repeat yourself twice" idiom
#define UREACT_FUNCTION_BODY( EXPR )                                                               \
    noexcept( noexcept( EXPR ) )->decltype( EXPR )                                                 \
    {                                                                                              \
        return ( EXPR );                                                                           \
    }

// Forward arg
#define UREACT_FWD( X ) std::forward<decltype( X )>( X )

// NOLINTNEXTLINE
#define UREACT_MAKE_NOOP_ITERATOR( ClassName )                                                     \
    ClassName& operator*()                                                                         \
    {                                                                                              \
        return *this;                                                                              \
    }                                                                                              \
    ClassName& operator++()                                                                        \
    {                                                                                              \
        return *this;                                                                              \
    }                                                                                              \
    ClassName operator++( int )                                                                    \
    {                                                                                              \
        return *this;                                                                              \
    }                                                                                              \
    static_assert( true ) /*enforce semicolon*/

UREACT_BEGIN_NAMESPACE

class context;

template <typename S>
class signal;

template <typename S>
class var_signal;

template <typename S, typename op_t>
class temp_signal;

template <typename E>
class events;

template <typename E>
class event_source;

template <typename... values_t>
class signal_pack;

template <typename E>
class event_range;

template <class F>
class closure;

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

// chaining of std::conditional_t  based on
// https://stackoverflow.com/questions/32785105/implementing-a-switch-type-trait-with-stdconditional-t-chain-calls/32785263#32785263

/*!
 * @brief Utility for using with select_t
 */
template <bool B, typename T>
struct condition
{
    static constexpr bool value = B;
    using type = T;
};

template <typename Head, typename... Tail>
struct select_impl : std::conditional_t<Head::value, Head, select_impl<Tail...>>
{};

template <typename T>
struct select_impl<T>
{
    using type = T;
};

template <bool B, typename T>
struct select_impl<condition<B, T>>
{
    // last one had better be true!
    static_assert( B, "!" );
    using type = T;
};

/*!
 * @brief Utility for selecting type based on several conditions
 *
 * Usage:
 *   template<class T>
 *   using foo =
 *      select_t<condition<std::is_convertible_v<T, A>, A>,
 *               condition<std::is_convertible_v<T, B>, B>,
 *               void>;
 * the same as
 *   template<class T>
 *   using foo =
 *      std::conditional_t<
 *          std::is_convertible_v<T, A>,
 *          A,
 *          std::conditional_t<
 *              std::is_convertible_v<T, B>,
 *              B,
 *              void>>;
 */
template <typename Head, typename... Tail>
using select_t = typename select_impl<Head, Tail...>::type;

/*!
 * @brief Helper for static assert
 */
template <typename...>
constexpr inline bool always_false = false;

/*!
 * @brief Helper class to mark failing of class match
 */
struct signature_mismatches;

} // namespace detail

/*!
 * @brief Return if type is signal or its inheritor
 */
template <typename T>
struct is_signal : detail::is_base_of_template<signal, T>
{};

/*!
 * @brief Helper variable template for is_signal
 */
template <typename T>
inline constexpr bool is_signal_v = is_signal<T>::value;

/*!
 * @brief Return if type is events or its inheritor
 */
template <typename T>
struct is_event : detail::is_base_of_template<events, T>
{};

/*!
 * @brief Helper variable template for is_event
 */
template <typename T>
inline constexpr bool is_event_v = is_event<T>::value;

/*!
 * @brief Return if type is closure
 */
template <typename T>
struct is_closure : detail::is_base_of_template<closure, T>
{};

/*!
 * @brief Helper variable template for closure
 */
template <typename T>
inline constexpr bool is_closure_v = is_closure<T>::value;

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
    first = detail::find_if_not( first, last, p );
    if( first == last )
    {
        return first;
    }

    for( forward_it i = std::next( first ); i != last; ++i )
    {
        if( p( *i ) )
        {
            detail::iter_swap( i, first );
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

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wfloat-equal"
#endif

template <typename L, class R, typename = void>
struct equality_comparable_with : std::false_type
{};

template <typename L, class R>
struct equality_comparable_with<L,
    R,
    std::void_t<decltype( std::declval<L>() == std::declval<R>() )>> : std::true_type
{};

template <typename L, class R>
inline constexpr bool equality_comparable_with_v = equality_comparable_with<L, R>::value;

template <typename L, typename R>
UREACT_WARN_UNUSED_RESULT bool equals( const L& lhs, const R& rhs )
{
    if constexpr( equality_comparable_with_v<L, R> )
    {
        return lhs == rhs;
    }
    else
    {
        // if operator == is not defined falling back to comparing addresses
        return &lhs == &rhs; // TODO: check in tests
    }
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

template <typename Cont, typename Value, typename = void>
struct has_push_back_method : std::false_type
{};

template <typename Cont, class Value>
struct has_push_back_method<Cont,
    Value,
    std::void_t<decltype( std::declval<Cont>().push_back( std::declval<Value>() ) )>>
    : std::true_type
{};

template <typename Cont, class Value>
inline constexpr bool has_push_back_method_v = has_push_back_method<Cont, Value>::value;

template <typename Cont, typename Value, typename = void>
struct has_insert_method : std::false_type
{};

template <typename Cont, class Value>
struct has_insert_method<Cont,
    Value,
    std::void_t<decltype( std::declval<Cont>().insert( std::declval<Value>() ) )>> : std::true_type
{};

template <typename Cont, class Value>
inline constexpr bool has_insert_method_v = has_insert_method<Cont, Value>::value;

/*!
 * @brief counter that counts down from N to 0
 *
 * prefix decrement operator decrements only to 0
 * example: for(countdown i{N}; i; --i )
 */
class countdown
{
public:
    explicit countdown( size_t value )
        : m_value( value )
    {}

    countdown& operator--()
    {
        m_value = dec( m_value );
        return *this;
    }

    const countdown operator--( int ) // NOLINT
    {
        countdown i{ m_value };
        m_value = dec( m_value );
        return i;
    }

    // checkable in boolean context
    explicit operator bool() const
    {
        return m_value;
    }

private:
    // decrement operator decrements only to 0
    [[nodiscard]] static size_t dec( const size_t value )
    {
        if( value == 0 ) // [[likely]]
            return 0;
        else
            return value - 1;
    }

    size_t m_value;
};

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

        UREACT_WARN_UNUSED_RESULT const std::vector<value_type>& next_values() const
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
    const auto p = detail::partition( m_queue_data.begin(),
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
    // node_base contains reference to context, and it will break if context lose its graph
    context_internals( context_internals&& ) noexcept = delete;
    context_internals& operator=( context_internals&& ) noexcept = delete;

    UREACT_WARN_UNUSED_RESULT react_graph& get_graph()
    {
        return *m_graph;
    }

    UREACT_WARN_UNUSED_RESULT const react_graph&
    get_graph() const // TODO: check in tests or completely remove
    {
        return *m_graph;
    }

    /// Perform several changes atomically
    template <typename F>
    void do_transaction( F&& func )
    {
        m_graph->do_transaction( std::forward<F>( func ) );
    }

private:
    std::unique_ptr<react_graph> m_graph;
};

// forward declaration
class node_base;

} // namespace detail

template <typename F>
void do_transaction( context& ctx, F&& func );

/*!
 * @brief Core class that connects all reactive nodes together.
 *
 *  Each signal and node belongs to a single ureact context.
 *  Signals from different contexts can't interact with each other.
 */
class context final : protected detail::context_internals
{
public:
    UREACT_WARN_UNUSED_RESULT bool operator==( const context& rsh ) const
    {
        return this == &rsh; // TODO: check in tests
    }

    UREACT_WARN_UNUSED_RESULT bool operator!=( const context& rsh ) const
    {
        return !( *this == rsh ); // TODO: check in tests
    }

private:
    friend class detail::node_base;

    template <typename F>
    friend void do_transaction( context& ctx, F&& func );

    /// Returns internals. Not intended to use in user code
    UREACT_WARN_UNUSED_RESULT friend context_internals& _get_internals( context& ctx )
    {
        return ctx;
    }
};

/*!
 * @brief Perform several changes atomically
 *
 *  The signature of func should be equivalent to:
 *  * void func()
 */
template <typename F>
void do_transaction( context& ctx, F&& func )
{
    static_assert( std::is_invocable_r_v<void, F>, "Transaction functions should be void()" );
    ctx.do_transaction( std::forward<F>( func ) );
}

/*!
 * @brief Closure objects used for partial application of reactive functions and chaining of algorithms
 *
 *  Closure objects take one reactive object as its only argument and may return a value.
 *  They are callable via the pipe operator: if C is a closure object and
 *  R is a reactive object, these two expressions are equivalent:
 *  * C(R)
 *  * R | C
 *
 *  Two closure objects can be chained by operator| to produce
 *  another closure object: if C and D are closure objects,
 *  then C | D is also a closure object if it is valid.
 *  The effect and validity of the operator() of the result is determined as follows:
 *  given a reactive object R, these two expressions are equivalent:
 *  * R | C | D // (R | C) | D
 *  * R | (C | D)
 *
 * @note similar to https://en.cppreference.com/w/cpp/ranges#Range_adaptor_closure_objects
 */
template <class F>
class closure
{
public:
    // TODO: add type specialization to closure (not concrete type, but signal/events), so we can write specialized operator overloads
    //       need to tag both input and output value. Also closure is single in, single out function. Or no output function
    explicit closure( F&& func )
        : m_func( std::move( func ) )
    {}

    template <typename... args_t, class = std::enable_if_t<std::is_invocable_v<F, args_t...>>>
    UREACT_WARN_UNUSED_RESULT auto operator()( args_t&&... args ) const
    {
        return m_func( std::forward<args_t>( args )... );
    }

private:
    F m_func;
};

/*!
 * @brief operator| overload for closure object
 *
 *  See @ref closure
 */
template <typename Arg,
    typename Closure,
    class = std::enable_if_t<is_closure_v<std::decay_t<Closure>>>>
UREACT_WARN_UNUSED_RESULT auto operator|( Arg&& arg, Closure&& closure_obj )
{
    if constexpr( is_closure_v<std::decay_t<Arg>> )
    {
        // chain two closures to make another one
        using FirstClosure = Arg;
        using SecondClosure = Closure;
        return closure{
            [first_closure = std::forward<FirstClosure>( arg ),
                second_closure = std::forward<SecondClosure>( closure_obj )]( auto&& source ) {
                using arg_t = decltype( source );
                return second_closure( first_closure( std::forward<arg_t>( source ) ) );
            } };
    }
    else
    {
        // apply arg to given closure and return its result
        return std::forward<Closure>( closure_obj )( std::forward<Arg>( arg ) );
    }
}

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

    UREACT_WARN_UNUSED_RESULT const react_graph&
    get_graph() const // TODO: check in tests or remove it completely
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

    explicit reactive_base( std::shared_ptr<node_t>&& node )
        : m_node( std::move( node ) )
    {}

    UREACT_WARN_UNUSED_RESULT bool is_valid() const
    {
        return m_node != nullptr;
    }

    UREACT_WARN_UNUSED_RESULT bool equals( const reactive_base& other ) const
    {
        return this->m_node == other.m_node;
    }

    UREACT_WARN_UNUSED_RESULT context& get_context() const
    {
        return m_node->get_context();
    }

protected:
    std::shared_ptr<node_t> m_node;

    template <typename node_t_>
    friend const std::shared_ptr<node_t_>& get_node_ptr( const reactive_base<node_t_>& node );
};

template <typename node_t>
UREACT_WARN_UNUSED_RESULT const std::shared_ptr<node_t>& get_node_ptr(
    const reactive_base<node_t>& node )
{
    return node.m_node;
}

} // namespace detail

//==================================================================================================
// [[section]] Signals
//==================================================================================================
namespace detail
{

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
class var_node final
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
    bool m_is_input_added = false; // TODO: replace 2 bools with enum class
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
            return m_func( eval( std::forward<T>( args ) )... );
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
class signal_op_node final : public signal_node<S>
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
        assert( !m_was_op_stolen && "Op was already stolen" );
        m_was_op_stolen = true;
        m_op.detach( *this );
        return std::move( m_op );
    }

    UREACT_WARN_UNUSED_RESULT bool was_op_stolen() const // TODO: check in tests
    {
        return m_was_op_stolen;
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

    template <typename node_t>
    explicit signal_base( std::shared_ptr<node_t>&& node )
        : signal_base::reactive_base( std::move( node ) )
    {}

protected:
    UREACT_WARN_UNUSED_RESULT const S& get_value() const
    {
        return this->m_node->value_ref();
    }

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

private:
    UREACT_WARN_UNUSED_RESULT auto get_var_node() const
    {
        return static_cast<var_node<S>*>( this->m_node.get() );
    }
};

} // namespace detail

/*!
 * @brief Reactive variable that can propagate its changes to dependents and react to changes of
 * its dependencies
 *
 *  A signal is a reactive variable that can propagate its changes to dependents
 *  and react to changes of its dependencies.
 *
 *  Instances of this class act as a proxies to signal nodes. It takes shared
 *  ownership of the node, so while it exists, the node will not be destroyed.
 *  Copy, move and assignment semantics are similar to std::shared_ptr.
 */
template <typename S>
class signal : public detail::signal_base<S>
{
protected:
    using node_t = detail::signal_node<S>;

public:
    /*!
     * @brief Alias to value type to use in metaprogramming
     */
    using value_t = S;

    /*!
     * @brief Default construct @ref signal
     *
     * Default constructed @ref signal is not attached to node, so it is not valid
     */
    signal() = default;

    /*!
     * @brief Construct from the given node
     */
    explicit signal( std::shared_ptr<node_t>&& node )
        : signal::signal_base( std::move( node ) )
    {}

    /*!
     * @brief Return value of linked node
     */
    UREACT_WARN_UNUSED_RESULT const S& get() const
    {
        assert( this->is_valid() && "Can't get value of signal not attached to a node" );
        return this->get_value();
    }

    /*!
     * @brief Return value of linked node
     */
    UREACT_WARN_UNUSED_RESULT const S& operator()() const
    {
        assert( this->is_valid() && "Can't get value of signal not attached to a node" );
        return this->get_value();
    }
};

/*!
 * @brief Source signals which values can be manually changed
 *
 *  This class extends the immutable signal interface with functions that support
 *  imperative value input. In the dataflow graph, input signals are sources.
 *  As such, they don't have any predecessors.
 */
template <typename S>
class var_signal final : public signal<S>
{
private:
    using node_t = detail::var_node<S>;

public:
    /*!
     * @brief Default construct @ref var_signal
     *
     * Default constructed @ref var_signal is not attached to node, so it is not valid.
     */
    var_signal() = default;

    /*!
     * @brief Construct a fully functional var signal
     *
     * @note replacing type of value with universal reference version prevents class template argument deduction
     */
    var_signal( context& context, const S& value )
        : var_signal::signal( std::make_shared<node_t>( context, value ) )
    {}

    /*!
     * @brief Construct a fully functional var signal
     *
     * @note replacing type of value with universal reference version prevents class template argument deduction
     */
    var_signal( context& context, S&& value )
        : var_signal::signal( std::make_shared<node_t>( context, std::move( value ) ) )
    {}

    /*!
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
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( new_value );
    }

    /*!
     * @brief Set new signal value
     *
     * Specialization of set(const S& new_value) for rvalue
     */
    void set( S&& new_value ) const
    {
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( std::move( new_value ) );
    }

    /*!
     * @brief Modify current signal value in-place
     *
     *  The signature of func should be equivalent to:
     *  * void func(const S&)
     *
     *  We can not compare the old and new values, we lose the ability to detect
     *  whether the data was actually changed. We always have to assume that
     *  it did and re-calculate dependent signals.
     */
    template <typename F>
    void modify( const F& func ) const
    {
        static_assert(
            std::is_invocable_r_v<void, F, S&>, "Modifier functions should be void(S&)" );
        assert( this->is_valid() && "Can't modify value of var_signal not attached to a node" );
        this->modify_value( func );
    }

    /*!
     * @brief Set new signal value
     *
     *  Operator version of set(const S& new_value)
     */
    void operator<<=( const S& new_value ) const
    {
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( new_value );
    }

    /*!
     * @brief Set new signal value
     *
     *  Operator version of set(S&& new_value)
     *
     *  Specialization of operator<<=(const S& new_value) for rvalue
     */
    void operator<<=( S&& new_value ) const
    {
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( std::move( new_value ) );
    }

    /*!
     * @brief Modify current signal value in-place
     *
     *  Operator version of modify(const F& func)
     */
    template <typename F, class = std::enable_if_t<std::is_invocable_v<F, S&>>>
    void operator<<=( const F& func ) const
    {
        static_assert(
            std::is_invocable_r_v<void, F, S&>, "Modifier functions should be void(S&)" );
        assert( this->is_valid() && "Can't modify value of var_signal not attached to a node" );
        this->modify_value( func );
    }
};

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
template <typename S, typename op_t>
class temp_signal final : public signal<S>
{
private:
    using node_t = detail::signal_op_node<S, op_t>;

public:
    /*!
     * @brief Construct a fully functional temp signal
     */
    template <typename... Args>
    explicit temp_signal( context& context, Args&&... args )
        : temp_signal::signal( std::make_shared<node_t>( context, std::forward<Args>( args )... ) )
    {}

    /*!
     * @brief Return internal operator, leaving node invalid
     */
    UREACT_WARN_UNUSED_RESULT op_t steal_op()
    {
        auto* node_ptr = static_cast<node_t*>( this->m_node.get() );
        return node_ptr->steal_op();
    }

    /*!
     * @brief Checks if internal operator was already stolen
     */
    UREACT_WARN_UNUSED_RESULT bool was_op_stolen() const // TODO: check in tests
    {
        auto* node_ptr = static_cast<node_t*>( this->m_node.get() );
        return node_ptr->was_op_stolen();
    }
};

/*!
 * @brief A wrapper type for a tuple of signal references
 *
 *  Created with @ref with()
 */
template <typename... values_t>
class signal_pack final
{
public:
    /*!
     * @brief Construct from signals
     */
    explicit signal_pack( const signal<values_t>&... deps )
        : data( std::tie( deps... ) )
    {}

    /*!
     * @brief The wrapped tuple
     */
    std::tuple<const signal<values_t>&...> data;
};

namespace detail
{

template <typename V, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto make_var_impl( context& context, V&& v )
{
    // TODO: use select_t to detect var_signal type and then construct it once
    if constexpr( is_signal_v<S> || is_event_v<S> )
    {
        using inner_t = typename S::value_t;
        if constexpr( is_signal_v<S> )
        {
            return var_signal<signal<inner_t>>{ context, std::forward<V>( v ) };
        }
        else if constexpr( is_event_v<S> )
        {
            return var_signal<events<inner_t>>{ context, std::forward<V>( v ) };
        }
    }
    else
    {
        return var_signal<S>{ context, std::forward<V>( v ) };
    }
}

} // namespace detail

/*!
 * @brief Create a new input signal node and links it to the returned make_var instance
 */
template <typename V>
UREACT_WARN_UNUSED_RESULT auto make_var( context& context, V&& value )
{
    return make_var_impl( context, std::forward<V>( value ) );
}

/*!
 * @brief Utility function to create a signal_pack from given signals
 *
 *  Creates a signal_pack from the signals passed as deps.
 *  Semantically, this is equivalent to std::tie.
 */
template <typename... values_t>
UREACT_WARN_UNUSED_RESULT auto with( const signal<values_t>&... deps )
{
    return signal_pack<values_t...>( deps... );
}

/*!
 * @brief Create a new signal node with value v = func(arg_pack.get(), ...).
 * This value is set on construction and updated when any args have changed
 *
 *  The signature of func should be equivalent to:
 *  * S func(const values_t& ...)
 */
template <typename... values_t,
    typename in_f,
    typename F = std::decay_t<in_f>,
    typename S = std::invoke_result_t<F, values_t...>,
    typename op_t = detail::function_op<S, F, detail::signal_node_ptr_t<values_t>...>>
UREACT_WARN_UNUSED_RESULT auto make_signal( const signal_pack<values_t...>& arg_pack, in_f&& func )
{
    context& context = std::get<0>( arg_pack.data ).get_context();

    auto node_builder = [&context, &func]( const signal<values_t>&... args ) {
        return temp_signal<S, op_t>{ context, std::forward<in_f>( func ), get_node_ptr( args )... };
    };

    return std::apply( node_builder, arg_pack.data );
}

/*!
 * @brief Create a new signal node with value v = func(arg.get()).
 * This value is set on construction and updated when arg have changed
 *
 *  The signature of func should be equivalent to:
 *  * S func(const value_t&)
 */
template <typename value_t, typename in_f>
UREACT_WARN_UNUSED_RESULT auto make_signal( const signal<value_t>& arg, in_f&& func )
{
    return make_signal( with( arg ), std::forward<in_f>( func ) );
}

/*!
 * @brief operator| overload to connect a signal to a function and return the resulting signal
 *
 *  The signature of func should be equivalent to:
 *  * S func(const value_t&)
 *
 *  the same as make_signal(arg, func)
 */
template <typename F, typename T, class = std::enable_if_t<is_signal_v<T>>>
UREACT_WARN_UNUSED_RESULT auto operator|( const T& arg, F&& func )
{
    return make_signal( arg, std::forward<F>( func ) );
}

/*!
 * @brief operator| overload to connect multiple signals to a function and return the resulting signal
 *
 *  The signature of func should be equivalent to:
 *  * S func(const values_t& ...)
 *
 *  the same as make_signal(with(args), func)
 */
template <typename F, typename... values_t>
UREACT_WARN_UNUSED_RESULT auto operator|( const signal_pack<values_t...>& arg_pack, F&& func )
{
    return make_signal( arg_pack, std::forward<F>( func ) );
}

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
    class = std::enable_if_t<is_signal_v<signal_t>>>
auto unary_operator_impl( const signal_t& arg )
{
    using val_t = typename signal_t::value_t;
    using F = functor_op<val_t>;
    using S = std::invoke_result_t<F, val_t>;
    using op_t = function_op<S, F, signal_node_ptr_t<val_t>>;
    return temp_signal<S, op_t>{ arg.get_context(), F(), get_node_ptr( arg ) };
}

template <template <typename> class functor_op, typename val_t, typename op_in_t>
auto unary_operator_impl( temp_signal<val_t, op_in_t>&& arg )
{
    using F = functor_op<val_t>;
    using S = std::invoke_result_t<F, val_t>;
    using op_t = function_op<S, F, op_in_t>;
    return temp_signal<S, op_t>{ arg.get_context(), F(), arg.steal_op() };
}

template <template <typename, typename> class functor_op,
    typename left_signal_t,
    typename right_signal_t,
    class = std::enable_if_t<is_signal_v<left_signal_t>>,
    class = std::enable_if_t<is_signal_v<right_signal_t>>>
auto binary_operator_impl( const left_signal_t& lhs, const right_signal_t& rhs )
{
    using left_val_t = typename left_signal_t::value_t;
    using right_val_t = typename right_signal_t::value_t;
    using F = functor_op<left_val_t, right_val_t>;
    using S = std::invoke_result_t<F, left_val_t, right_val_t>;
    using op_t = function_op<S, F, signal_node_ptr_t<left_val_t>, signal_node_ptr_t<right_val_t>>;

    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return temp_signal<S, op_t>{ context, F(), get_node_ptr( lhs ), get_node_ptr( rhs ) };
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

    return temp_signal<S, op_t>{
        context, F( std::forward<right_val_in_t>( rhs ) ), get_node_ptr( lhs ) };
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

    return temp_signal<S, op_t>{
        context, F( std::forward<left_val_in_t>( lhs ) ), get_node_ptr( rhs ) };
}

template <template <typename, typename> class functor_op,
    typename left_val_t,
    typename left_op_t,
    typename right_val_t,
    typename right_op_t>
auto binary_operator_impl(
    temp_signal<left_val_t, left_op_t>&& lhs, temp_signal<right_val_t, right_op_t>&& rhs )
{
    using F = functor_op<left_val_t, right_val_t>;
    using S = std::invoke_result_t<F, left_val_t, right_val_t>;
    using op_t = function_op<S, F, left_op_t, right_op_t>;

    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return temp_signal<S, op_t>{ context, F(), lhs.steal_op(), rhs.steal_op() };
}

template <template <typename, typename> class functor_op,
    typename left_val_t,
    typename left_op_t,
    typename right_signal_t,
    class = std::enable_if_t<is_signal_v<right_signal_t>>>
auto binary_operator_impl( temp_signal<left_val_t, left_op_t>&& lhs, const right_signal_t& rhs )
{
    using right_val_t = typename right_signal_t::value_t;
    using F = functor_op<left_val_t, right_val_t>;
    using S = std::invoke_result_t<F, left_val_t, right_val_t>;
    using op_t = function_op<S, F, left_op_t, signal_node_ptr_t<right_val_t>>;

    context& context = rhs.get_context();

    return temp_signal<S, op_t>{ context, F(), lhs.steal_op(), get_node_ptr( rhs ) };
}

template <template <typename, typename> class functor_op,
    typename left_signal_t,
    typename right_val_t,
    typename right_op_t,
    class = std::enable_if_t<is_signal_v<left_signal_t>>>
auto binary_operator_impl( const left_signal_t& lhs, temp_signal<right_val_t, right_op_t>&& rhs )
{
    using left_val_t = typename left_signal_t::value_t;
    using F = functor_op<left_val_t, right_val_t>;
    using S = std::invoke_result_t<F, left_val_t, right_val_t>;
    using op_t = function_op<S, F, signal_node_ptr_t<left_val_t>, right_op_t>;

    context& context = lhs.get_context();

    return temp_signal<S, op_t>{ context, F(), get_node_ptr( lhs ), rhs.steal_op() };
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

    return temp_signal<S, op_t>{
        context, F( std::forward<right_val_in_t>( rhs ) ), lhs.steal_op() };
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

    return temp_signal<S, op_t>{ context, F( std::forward<left_val_in_t>( lhs ) ), rhs.steal_op() };
}

} // namespace detail

#define UREACT_DECLARE_UNARY_OP_FUNCTOR( op, name )                                                \
    namespace detail                                                                               \
    {                                                                                              \
    template <typename V>                                                                          \
    struct op_functor_##name                                                                       \
    {                                                                                              \
        UREACT_WARN_UNUSED_RESULT auto operator()( const V& v ) const UREACT_FUNCTION_BODY( op v ) \
    };                                                                                             \
    } /* namespace detail */

#define UREACT_DECLARE_UNARY_OP( op, name )                                                        \
    template <typename signal_t,                                                                   \
        template <typename> class functor_op = detail::op_functor_##name,                          \
        class = std::enable_if_t<is_signal_v<std::decay_t<signal_t>>>>                             \
    UREACT_WARN_UNUSED_RESULT auto operator op( signal_t&& arg )                                   \
        UREACT_FUNCTION_BODY( detail::unary_operator_impl<functor_op>( UREACT_FWD( arg ) ) )

#define UREACT_DECLARE_UNARY_OPERATOR( op, name )                                                  \
    UREACT_DECLARE_UNARY_OP_FUNCTOR( op, name )                                                    \
    UREACT_DECLARE_UNARY_OP( op, name )

#define UREACT_DECLARE_BINARY_OP_FUNCTOR( op, name )                                               \
    namespace detail                                                                               \
    {                                                                                              \
    template <typename L, typename R>                                                              \
    struct op_functor_##name                                                                       \
    {                                                                                              \
        UREACT_WARN_UNUSED_RESULT auto operator()( const L& lhs, const R& rhs ) const              \
            UREACT_FUNCTION_BODY( lhs op rhs )                                                     \
    };                                                                                             \
    } /* namespace detail */

#define UREACT_DECLARE_BINARY_OP( op, name )                                                       \
    template <typename lhs_t,                                                                      \
        typename rhs_t,                                                                            \
        template <typename, typename> class functor_op = detail::op_functor_##name,                \
        class = std::enable_if_t<                                                                  \
            std::disjunction_v<is_signal<std::decay_t<lhs_t>>, is_signal<std::decay_t<rhs_t>>>>>   \
    UREACT_WARN_UNUSED_RESULT auto operator op( lhs_t&& lhs, rhs_t&& rhs ) /*                */    \
        UREACT_FUNCTION_BODY(                                                                      \
            detail::binary_operator_impl<functor_op>( UREACT_FWD( lhs ), UREACT_FWD( rhs ) ) )

#define UREACT_DECLARE_BINARY_OPERATOR( op, name )                                                 \
    UREACT_DECLARE_BINARY_OP_FUNCTOR( op, name )                                                   \
    UREACT_DECLARE_BINARY_OP( op, name )

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wunknown-warning-option"
#    pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"
#endif

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

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic pop
#endif

#undef UREACT_DECLARE_UNARY_OPERATOR
#undef UREACT_DECLARE_UNARY_OP_FUNCTOR
#undef UREACT_DECLARE_UNARY_OP
#undef UREACT_DECLARE_BINARY_OPERATOR
#undef UREACT_DECLARE_BINARY_OP_FUNCTOR
#undef UREACT_DECLARE_BINARY_OP

//==================================================================================================
// [[section]] Events
//==================================================================================================

/*!
 * @brief This class is used as value type of unit streams, which emit events without any value other than the fact that they occurred
 *
 *  See std::monostate https://en.cppreference.com/w/cpp/utility/variant/monostate
 */
struct unit
{};

// clang-format off
constexpr bool operator==(unit, unit) noexcept { return true; }
constexpr bool operator!=(unit, unit) noexcept { return false; }
constexpr bool operator< (unit, unit) noexcept { return false; }
constexpr bool operator> (unit, unit) noexcept { return false; }
constexpr bool operator<=(unit, unit) noexcept { return true; }
constexpr bool operator>=(unit, unit) noexcept { return true; }
//constexpr std::strong_ordering operator<=>(unit, unit) noexcept { return std::strong_ordering::equal; }
// clang-format on

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
class event_source_node final
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
class event_op_node final : public event_stream_node<E>
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

    UREACT_WARN_UNUSED_RESULT op_t steal_op() // TODO: check in tests
    {
        assert( !m_was_op_stolen && "Op was already stolen" );
        m_was_op_stolen = true;
        m_op.detach( *this );
        return std::move( m_op );
    }

    UREACT_WARN_UNUSED_RESULT bool was_op_stolen() const // TODO: check in tests
    {
        return m_was_op_stolen;
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

    UREACT_MAKE_COPYABLE( event_stream_base );
    UREACT_MAKE_MOVABLE( event_stream_base );

    template <typename node_t>
    explicit event_stream_base( std::shared_ptr<node_t>&& node )
        : event_stream_base::reactive_base( std::move( node ) )
    {}

private:
    UREACT_WARN_UNUSED_RESULT auto get_event_source_node() const -> event_source_node<E>*
    {
        return static_cast<event_source_node<E>*>( this->m_node.get() );
    }

protected:
    template <typename T>
    void emit_event( T&& e ) const
    {
        auto node_ptr = get_event_source_node();
        auto& graph_ref = node_ptr->get_graph();

        graph_ref.push_input(
            node_ptr, [node_ptr, &e] { node_ptr->emit_value( std::forward<T>( e ) ); } );
    }
};

} // namespace detail

/*!
 * @brief Reactive event stream class
 *
 *  An instance of this class acts as a proxy to an event stream node.
 *  It takes shared ownership of the node, so while it exists, the node will not be destroyed.
 *  Copy, move and assignment semantics are similar to std::shared_ptr.
 */
template <typename E = unit>
class events : public detail::event_stream_base<E>
{
private:
    using node_t = detail::event_stream_node<E>;

public:
    /*!
     * @brief Alias to value type to use in metaprogramming
     */
    using value_t = E;

    /*!
     * @brief Default construct @ref events
     *
     * Default constructed @ref events is not attached to node, so it is not valid
     */
    events() = default;

    /*!
     * @brief Construct from the given node
     */
    explicit events( std::shared_ptr<node_t>&& node )
        : events::event_stream_base( std::move( node ) )
    {}
};

/*!
 * @brief @ref events that support imperative input
 *
 *  An event source extends the immutable @ref events interface with functions that support imperative input.
 */
template <typename E = unit>
class event_source final : public events<E>
{
private:
    using node_t = detail::event_source_node<E>;

public:
    class iterator;

    /*!
     * @brief Default construct @ref event_source
     *
     * Default constructed @ref event_source is not attached to node, so it is not valid
     */
    event_source() = default;

    /*!
     * @brief Construct a fully functional event source
     */
    explicit event_source( context& context )
        : event_source::events( std::make_shared<node_t>( context ) )
    {}

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * If emit() was called inside of a transaction function, it will return after
     * the event has been queued and propagation is delayed until the transaction
     * function returns.
     * Otherwise, propagation starts immediately and emit() blocks until it’s done.
     */
    void emit( const E& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( e );
    }

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * Specialization of emit(const E& e) for rvalue
     */
    void emit( E&& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( std::move( e ) );
    }

    /*!
     * @brief Adds unit to the queue of outgoing events of the linked event source node
     *
     * Specialization of emit(const E& e) that allows to omit e value, when the emitted value is always @ref unit
     */
    void emit() const
    {
        static_assert( std::is_same_v<E, unit>, "Can't emit on non unit stream" );
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( unit{} );
    }

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * Function object version of emit(const E& e)
     */
    void operator()( const E& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( e );
    }

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * Function object version of emit(E&& e)
     */
    void operator()( E&& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( std::move( e ) );
    }

    /*!
     * @brief Adds unit to the queue of outgoing events of the linked event source node
     *
     * Function object version of emit()
     *
     */
    void operator()() const
    {
        static_assert( std::is_same_v<E, unit>, "Can't emit on non unit stream" );
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( unit{} );
    }

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * Stream version of emit(const E& e)
     *
     * @note chaining multiple events in a single statement will not execute them in a single transaction
     */
    const event_source& operator<<( const E& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( e );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events of the linked event source node
     *
     * Stream version of emit(E&& e)
     *
     * Specialization of operator<<(const E& e) for rvalue
     *
     * @note chaining multiple events in a single statement will not execute them in a single transaction
     */
    const event_source& operator<<( E&& e ) const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        this->emit_event( std::move( e ) );
        return *this;
    }

    /*!
     * @brief Returns std compatible iterator for emitting using std algorithms like std::copy()
     */
    auto begin() const
    {
        assert( this->is_valid() && "Can't emit from event_source not attached to a node" );
        return iterator{ *this };
    }
};

/*!
 * @brief Iterator for compatibility with std algorithms
 */
template <typename E>
class event_source<E>::iterator final
{
public:
    UREACT_MAKE_NOOP_ITERATOR( iterator ); // NOLINT

    using iterator_category = std::output_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = E;
    using pointer = value_type*;
    using reference = value_type&;

    /*!
     * @brief Constructor
     */
    explicit iterator( const event_source& parent )
        : m_parent( &parent )
    {}

    /*!
     * @brief Adds e to the queue of outgoing events
     */
    iterator& operator=( const E& e )
    {
        m_parent->emit_event( e );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     */
    iterator& operator=( E&& e )
    {
        m_parent->emit_event( std::move( e ) );
        return *this;
    }

private:
    const event_source* m_parent;
};

/*!
 * @brief Represents a range of events. It it serves as an adaptor to the underlying event container of a source node
 *
 *  An instance of event_range holds a reference to the wrapped container and selectively exposes functionality
 *  that allows to iterate over its events without modifying it.
 *
 *  TODO: think about making values movable, so values would be processed more efficiently
 */
template <typename E = unit>
class event_range final
{
public:
    using const_iterator = typename std::vector<E>::const_iterator;
    using const_reverse_iterator = typename std::vector<E>::const_reverse_iterator;
    using size_type = typename std::vector<E>::size_type;

    /*!
     * @brief Constructor
     */
    explicit event_range( const std::vector<E>& data )
        : m_data( data )
    {}

    /*!
     * @brief Returns a random access const iterator to the beginning
     */
    UREACT_WARN_UNUSED_RESULT const_iterator begin() const
    {
        return m_data.begin();
    }

    /*!
     * @brief Returns a random access const iterator to the end
     */
    UREACT_WARN_UNUSED_RESULT const_iterator end() const
    {
        return m_data.end();
    }

    /*!
     * @brief Returns a reverse random access const iterator to the beginning
     */
    UREACT_WARN_UNUSED_RESULT const_reverse_iterator rbegin() const
    {
        return m_data.rbegin();
    }

    /*!
     * @brief Returns a reverse random access const iterator to the end
     */
    UREACT_WARN_UNUSED_RESULT const_reverse_iterator rend() const // TODO: check in tests
    {
        return m_data.rend();
    }

    /*!
     * @brief Returns the number of events
     */
    UREACT_WARN_UNUSED_RESULT size_type size() const
    {
        return m_data.size();
    }

    /*!
     * @brief Checks whether the container is empty
     */
    UREACT_WARN_UNUSED_RESULT bool empty() const // TODO: check in tests
    {
        return m_data.empty();
    }

private:
    const std::vector<E>& m_data;
};

/*!
 * @brief Represents output stream of events.
 *
 *  It is std::back_insert_iterator analog, but not depending on heavy <iterator> header.
 *  Additionally to std::back_insert_iterator interface it provides emit() methods like event_stream has
 */
template <typename E>
class event_emitter final
{
public:
    using container_type = std::vector<E>;
    using iterator_category = std::output_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = E;
    using pointer = value_type*;
    using reference = value_type&;

    UREACT_MAKE_NOOP_ITERATOR( event_emitter );

    /*!
     * @brief Constructor
     */
    explicit event_emitter( container_type& container )
        : m_container( &container )
    {}

    /*!
     * @brief Adds e to the queue of outgoing events
     *
     * Iterator assign version of emit(const E& e). Prefer not to use it manually
     */
    event_emitter& operator=( const E& e ) // TODO: check in tests
    {
        m_container->push_back( e );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     *
     * Iterator assign version of emit(E&& e). Prefer not to use it manually
     *
     * Specialization of operator=(const E& e) for rvalue
     */
    event_emitter& operator=( E&& e ) // TODO: check in tests
    {
        m_container->push_back( std::move( e ) );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     *
     * If emit() was called inside of a transaction function, it will return after
     * the event has been queued and propagation is delayed until the transaction
     * function returns.
     * Otherwise, propagation starts immediately and emit() blocks until it’s done.
     */
    void emit( const E& e )
    {
        m_container->push_back( e );
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     *
     * Specialization of emit(const E& e) for rvalue
     */
    void emit( E&& e )
    {
        m_container->push_back( std::move( e ) );
    }

    /*!
     * @brief Adds unit to the queue of outgoing events
     *
     * Specialization of emit(const E& e) that allows to omit e value, when the emitted value is always @ref unit
     */
    void emit() // TODO: check in tests
    {
        static_assert( std::is_same_v<E, unit>, "Can't emit on non unit stream" );
        m_container->push_back( unit{} );
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     *
     * Function object version of emit(const E& e)
     */
    void operator()( const E& e ) // TODO: check in tests
    {
        m_container->push_back( e );
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     *
     * Function object version of emit(E&& e)
     */
    void operator()( E&& e ) // TODO: check in tests
    {
        m_container->push_back( std::move( e ) );
    }

    /*!
     * @brief Adds unit to the queue of outgoing events
     *
     * Function object version of emit()
     *
     */
    void operator()() // TODO: check in tests
    {
        static_assert( std::is_same_v<E, unit>, "Can't emit on non unit stream" );
        m_container->push_back( unit{} );
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     *
     * Stream version of emit(const E& e)
     */
    event_emitter& operator<<( const E& e ) // TODO: check in tests
    {
        m_container->push_back( e );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     *
     * Stream version of emit(E&& e)
     *
     * Specialization of operator<<(const E& e) for rvalue
     */
    event_emitter& operator<<( E&& e )
    {
        m_container->push_back( std::move( e ) );
        return *this;
    }

private:
    container_type* m_container;
};

namespace detail
{

template <typename E, typename... deps_t>
class event_merge_op : public reactive_op_base<deps_t...>
{
public:
    template <typename... deps_in_t>
    explicit event_merge_op( deps_in_t&&... deps )
        : event_merge_op::reactive_op_base( dont_move(), std::forward<deps_in_t>( deps )... )
    {}

    event_merge_op( event_merge_op&& other ) noexcept // TODO: check in tests
        : event_merge_op::reactive_op_base( std::move( other ) )
    {}

    template <typename turn_t_, typename collector_t>
    void collect( const turn_t_& turn, const collector_t& collector ) const
    {
        apply( collect_functor<turn_t_, collector_t>( turn, collector ), this->m_deps );
    }

    template <typename turn_t_, typename collector_t, typename functor_t>
    void collect_rec( const functor_t& functor ) const // TODO: check in tests
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
        void collect( const T& op ) const // TODO: check in tests
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

template <typename in_t, typename out_t, typename func_t, typename... dep_values_t>
class event_processing_node final : public event_stream_node<out_t>
{
public:
    template <typename F>
    event_processing_node( context& context,
        const std::shared_ptr<event_stream_node<in_t>>& source,
        F&& func,
        const std::shared_ptr<signal_node<dep_values_t>>&... deps )
        : event_processing_node::event_stream_node( context )
        , m_source( source )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps... )
    {
        this->get_graph().on_node_attach( *this, *source );
        ( this->get_graph().on_node_attach( *this, *deps ), ... );
    }

    ~event_processing_node() override
    {
        this->get_graph().on_node_detach( *this, *m_source );

        apply( detach_functor<event_processing_node, std::shared_ptr<signal_node<dep_values_t>>...>(
                   *this ),
            m_deps );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );
        // Update of this node could be triggered from deps,
        // so make sure source doesn't contain events from last turn
        m_source->set_current_turn( turn );

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
class event_zip_node final : public event_stream_node<std::tuple<values_t...>>
{
public:
    explicit event_zip_node(
        context& context, const std::shared_ptr<event_stream_node<values_t>>&... sources )
        : event_zip_node::event_stream_node( context )
        , m_slots( sources... )
    {
        ( this->get_graph().on_node_attach( *this, *sources ), ... );
    }

    ~event_zip_node() override
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
        explicit slot( const std::shared_ptr<event_stream_node<T>>& source )
            : source( source )
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

template <typename out_t, typename in_t, typename Op, typename... dep_values_t>
UREACT_WARN_UNUSED_RESULT auto process_impl(
    const events<in_t>& source, const signal_pack<dep_values_t...>& dep_pack, Op&& op )
    -> events<out_t>
{
    using F = std::decay_t<Op>;

    context& context = source.get_context();

    auto node_builder = [&context, &source, &op]( const signal<dep_values_t>&... deps ) {
        return events<out_t>(
            std::make_shared<event_processing_node<in_t, out_t, F, dep_values_t...>>( context,
                get_node_ptr( source ),
                std::forward<Op>( op ),
                get_node_ptr( deps )... ) );
    };

    return std::apply( node_builder, dep_pack.data );
}

} // namespace detail

/*!
 * @brief Create a new event source node and links it to the returned event_source instance
 *
 *  Event value type E has to be specified explicitly. It would be unit if it is omitted.
 */
template <typename E = unit>
UREACT_WARN_UNUSED_RESULT auto make_source( context& context ) -> event_source<E>
{
    return event_source<E>{ context };
}

/*!
 * @brief Emit all events in source1, ... sources
 *
 *  @warning Not to be confused with std::merge() or ranges::merge()
 */
template <typename source_t, typename... sources_t, typename E = source_t>
UREACT_WARN_UNUSED_RESULT auto merge(
    const events<source_t>& source1, const events<sources_t>&... sources ) -> events<E>
{
    static_assert( sizeof...( sources_t ) > 0, "merge: 2+ arguments are required" );

    using op_t = detail::event_merge_op<E,
        detail::event_stream_node_ptr_t<source_t>,
        detail::event_stream_node_ptr_t<sources_t>...>;

    context& context = source1.get_context();
    return events<E>( std::make_shared<detail::event_op_node<E, op_t>>(
        context, get_node_ptr( source1 ), get_node_ptr( sources )... ) );
}

/*!
 * @brief Create a new event stream by batch processing events from other stream
 *
 *  op is called with all events range from source in current turn.
 *  New events are emitted through "out".
 *  Synchronized values of signals in dep_pack are passed to op as additional arguments.
 *
 *  The signature of op should be equivalent to:
 *  * bool op(event_range<in_t> range, event_emitter<out_t> out, const deps_t& ...)
 *
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 *  @note The type of outgoing events T has to be specified explicitly, i.e. process<T>(src, with(deps), op)
 */
template <typename out_t, typename in_t, typename Op, typename... deps_t>
UREACT_WARN_UNUSED_RESULT auto process(
    const events<in_t>& source, const signal_pack<deps_t...>& dep_pack, Op&& op ) -> events<out_t>
{
    return detail::process_impl<out_t>( source, dep_pack, std::forward<Op>( op ) );
}

/*!
 * @brief Create a new event stream by batch processing events from other stream
 *
 *  Version without synchronization with additional signals
 *
 *  See process(const events<in_t>& source, const signal_pack<deps_t...>& dep_pack, Op&& op)
 */
template <typename out_t, typename in_t, typename Op>
UREACT_WARN_UNUSED_RESULT auto process( const events<in_t>& source, Op&& op ) -> events<out_t>
{
    return detail::process_impl<out_t>( source, signal_pack<>(), std::forward<Op>( op ) );
}

/*!
 * @brief Curried version of process(const events<in_t>& source, Op&& op) algorithm used for "pipe" syntax
 */
template <typename out_t, typename Op>
UREACT_WARN_UNUSED_RESULT auto process( Op&& op )
{
    return closure{ [op = std::forward<Op>( op )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return process<out_t>( std::forward<arg_t>( source ), op );
    } };
}

/*!
 * @brief Create a new event stream that filters events from other stream
 *
 *  For every event e in source, emit e if pred(e, deps...) == true.
 *  Synchronized values of signals in dep_pack are passed to op as additional arguments.
 *
 *  The signature of pred should be equivalent to:
 *  * bool pred(const E&, const deps_t& ...)
 *
 *  Semantically equivalent of ranges::filter
 *
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename E, typename Pred, typename... dep_values_t>
UREACT_WARN_UNUSED_RESULT auto filter(
    const events<E>& source, const signal_pack<dep_values_t...>& dep_pack, Pred&& pred )
    -> events<E>
{
    using F = std::decay_t<Pred>;
    using result_t = std::invoke_result_t<F, E, dep_values_t...>;
    static_assert(
        std::is_same_v<result_t, bool>, "Filter function result should be exactly bool" );

    return detail::process_impl<E>( source,
        dep_pack, //
        [pred = std::forward<Pred>( pred )](
            event_range<E> range, event_emitter<E> out, const auto... deps ) mutable {
            for( const auto& e : range )
                if( pred( e, deps... ) )
                    out.emit( e );
        } );
}

/*!
 * @brief Create a new event stream that filters events from other stream
 *
 *  Version without synchronization with additional signals
 *
 *  See filter(const events<E>& source, const signal_pack<deps_t...>& dep_pack, Pred&& pred)
 */
template <typename E, typename Pred>
UREACT_WARN_UNUSED_RESULT auto filter( const events<E>& source, Pred&& pred ) -> events<E>
{
    return filter( source, signal_pack<>(), std::forward<Pred>( pred ) );
}

/*!
 * @brief Curried version of filter(const events<E>& source, Pred&& pred) algorithm used for "pipe" syntax
 */
template <typename Pred>
UREACT_WARN_UNUSED_RESULT auto filter( Pred&& pred )
{
    return closure{ [pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return filter( std::forward<arg_t>( source ), pred );
    } };
}

/*!
 * @brief Create a new event stream that transforms events from other stream
 *
 *  For every event e in source, emit t = func(e, deps...).
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of func should be equivalent to:
 *  * T func(const E&, const deps_t& ...)
 *
 *  Semantically equivalent of ranges::transform
 *
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename in_t,
    typename F,
    typename... deps_t,
    typename out_t = std::invoke_result_t<F, in_t, deps_t...>>
UREACT_WARN_UNUSED_RESULT auto transform(
    const events<in_t>& source, const signal_pack<deps_t...>& dep_pack, F&& func ) -> events<out_t>
{
    return detail::process_impl<out_t>( source,
        dep_pack, //
        [func = std::forward<F>( func )](
            event_range<in_t> range, event_emitter<out_t> out, const auto... deps ) mutable {
            for( const auto& e : range )
                out.emit( func( e, deps... ) );
        } );
}

/*!
 * @brief Create a new event stream that transforms events from other stream
 *
 *  Version without synchronization with additional signals
 *
 *  See transform(const events<in_t>& source, const signal_pack<deps_t...>& dep_pack, F&& func)
 */
template <typename in_t, typename F, typename out_t = std::invoke_result_t<F, in_t>>
UREACT_WARN_UNUSED_RESULT auto transform( const events<in_t>& source, F&& func ) -> events<out_t>
{
    return transform( source, signal_pack<>(), std::forward<F>( func ) );
}

/*!
 * @brief Curried version of transform(const events<in_t>& source, f_in_t&& func) algorithm used for "pipe" syntax
 */
template <typename F>
UREACT_WARN_UNUSED_RESULT auto transform( F&& func )
{
    return closure{ [func = std::forward<F>( func )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return transform( std::forward<arg_t>( source ), func );
    } };
}

/*!
 * @brief Skips first N elements from the source stream
 *
 *  Semantically equivalent of std::ranges::views::drop
 */
template <typename E, typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT auto drop( const events<E>& source, const N count )
{
    assert( count >= 0 );
    return filter( source,                                        //
        [i = detail::countdown( count )]( const auto& ) mutable { //
            return !bool( i-- );
        } );
}

/*!
 * @brief Curried version of drop(const events<E>& source, const size_t count) algorithm used for "pipe" syntax
 */
template <typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT auto drop( const N count )
{
    assert( count >= 0 );
    return closure{ [count]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return drop( std::forward<arg_t>( source ), count );
    } };
}

/*!
 * @brief Keeps first N elements from the source stream
 *
 *  Semantically equivalent of std::ranges::views::take
 */
template <typename E, typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT auto take( const events<E>& source, const N count )
{
    assert( count >= 0 );
    return filter( source,                                        //
        [i = detail::countdown( count )]( const auto& ) mutable { //
            return bool( i-- );
        } );
}

/*!
 * @brief Curried version of take(const events<E>& source, const size_t count) algorithm used for "pipe" syntax
 */
template <typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT auto take( const N count )
{
    assert( count >= 0 );
    return closure{ [count]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return take( std::forward<arg_t>( source ), count );
    } };
}

/*!
 * @brief Take only the first element from another stream
 *
 *  The same as take(1)
 */
template <typename E>
UREACT_WARN_UNUSED_RESULT auto once( const events<E>& source )
{
    return take( source, 1 );
}

/*!
 * @brief Curried version of once(const events<E>& source) algorithm used for "pipe" syntax
 */
UREACT_WARN_UNUSED_RESULT inline auto once()
{
    return take( 1 );
}

/*!
 * @brief Skips the first elements of the source stream that satisfy the predicate
 *
 *  Takes events beginning at the first for which the predicate returns false.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of pred should be equivalent to:
 *  * bool func(const E&, const deps_t& ...)
 */
template <typename E, typename... deps_t, typename Pred>
UREACT_WARN_UNUSED_RESULT auto drop_while(
    const events<E>& source, const signal_pack<deps_t...>& dep_pack, Pred&& pred )
{
    auto dropper_while = [passed = false, pred = std::forward<Pred>( pred )](
                             const auto& e, const auto... deps ) mutable {
        passed = passed || !pred( e, deps... );
        return passed;
    };

    return filter( source, dep_pack, dropper_while );
}

/*!
 * @brief Skips the first elements of the source stream that satisfy the unary predicate
 *
 *  Takes events beginning at the first for which the predicate returns false.
 *  Semantically equivalent of std::ranges::views::drop_while
 */
template <typename E, typename Pred>
UREACT_WARN_UNUSED_RESULT auto drop_while( const events<E>& source, Pred&& pred )
{
    return drop_while( source, signal_pack<>(), std::forward<Pred>( pred ) );
}

/*!
 * @brief Curried version of drop_while(const events<E>& source, Pred&& pred) algorithm used for "pipe" syntax
 */
template <typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto drop_while( Pred&& pred )
{
    return closure{ [pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return drop_while( std::forward<arg_t>( source ), pred );
    } };
}

/*!
 * @brief Keeps the first elements of the source stream that satisfy the predicate
 *
 *  Keeps events from the source stream, starting at the beginning and ending
 *  at the first element for which the predicate returns false.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of pred should be equivalent to:
 *  * bool func(const E&, const deps_t& ...)
 */
template <typename E, typename... deps_t, typename Pred>
UREACT_WARN_UNUSED_RESULT auto take_while(
    const events<E>& source, const signal_pack<deps_t...>& dep_pack, Pred&& pred )
{
    auto taker_while = [passed = true, pred = std::forward<Pred>( pred )](
                           const auto& e, const auto... deps ) mutable {
        passed = passed && pred( e, deps... );
        return passed;
    };

    return filter( source, dep_pack, taker_while );
}

/*!
 * @brief Keeps the first elements of the source stream that satisfy the unary predicate
 *
 *  Keeps events from the source stream, starting at the beginning and ending
 *  at the first element for which the predicate returns false.
 *  Semantically equivalent of std::ranges::views::take_while
 */
template <typename E, typename Pred>
UREACT_WARN_UNUSED_RESULT auto take_while( const events<E>& source, Pred&& pred )
{
    return take_while( source, signal_pack<>(), std::forward<Pred>( pred ) );
}

/*!
 * @brief Curried version of take_while(const events<E>& source, Pred&& pred) algorithm used for "pipe" syntax
 */
template <typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto take_while( Pred&& pred )
{
    return closure{ [pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return take_while( std::forward<arg_t>( source ), pred );
    } };
}

/*!
 * @brief Filter out all except the first element from every consecutive group of equivalent elements
 *
 *  In other words: removes consecutive (adjacent) duplicates
 *
 *  Semantically equivalent of std::unique
 */
template <typename E>
UREACT_WARN_UNUSED_RESULT inline auto unique( const events<E>& source )
{
    return filter( source, [first = true, prev = E{}]( const E& e ) mutable {
        const bool pass = first || e != prev;
        first = false;
        prev = e;
        return pass;
    } );
}

/*!
 * @brief Curried version of unique(const events<E>& source) algorithm used for "pipe" syntax
 */
UREACT_WARN_UNUSED_RESULT inline auto unique()
{
    return closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return unique( std::forward<arg_t>( source ) );
    } };
}

/*!
 * @brief Emit a tuple (e1,…,eN) for each complete set of values for sources 1...N
 *
 *  Each source slot has its own unbounded buffer queue that persistently stores incoming events.
 *  For as long as all queues are not empty, one value is popped from each and emitted together as a tuple.
 *
 *  Semantically equivalent of ranges::zip
 */
template <typename arg_t, typename... args_t>
UREACT_WARN_UNUSED_RESULT auto zip( const events<arg_t>& arg1, const events<args_t>&... args )
    -> events<std::tuple<arg_t, args_t...>>
{
    static_assert( sizeof...( args_t ) >= 1, "zip: 2+ arguments are required" );

    context& context = arg1.get_context();
    return events<std::tuple<arg_t, args_t...>>(
        std::make_shared<detail::event_zip_node<arg_t, args_t...>>(
            context, get_node_ptr( arg1 ), get_node_ptr( args )... ) );
}

/*!
 * @brief Utility function to transform any event stream into a unit stream
 *
 *  Emits a unit for any event that passes source
 */
template <typename E>
UREACT_WARN_UNUSED_RESULT auto unify( const events<E>& source )
{
    return transform( source, []( const auto& ) { return unit{}; } );
}

/*!
 * @brief Curried version of unify(events_t&& source) algorithm used for "pipe" syntax
 */
UREACT_WARN_UNUSED_RESULT inline auto unify()
{
    return closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return unify( std::forward<arg_t>( source ) );
    } };
}

//==================================================================================================
// [[section]] Observers
//==================================================================================================

/*!
 * @brief Observer functions can return values of this type to control further processing.
 */
enum class observer_action
{
    next,           ///< Continue observing
    stop_and_detach ///< Stop observing
};

// forward declaration
class observer;

namespace detail
{

template <typename in_f, typename S>
auto observe_signal_impl( const signal<S>& subject, in_f&& func ) -> observer;

template <typename f_in_t, typename E, typename... deps_t>
auto observe_events_impl(
    const events<E>& subject, const signal_pack<deps_t...>& dep_pack, f_in_t&& func ) -> observer;

} // namespace detail

/*!
 * @brief Shared pointer like object that holds a strong reference to the observed subject
 *
 *  An instance of this class provides a unique handle to an observer which can
 *  be used to detach it explicitly. It also holds a strong reference to
 *  the observed subject, so while it exists the subject and therefore
 *  the observer will not be destroyed.
 *
 *  If the handle is destroyed without calling detach(), the lifetime of
 *  the observer is tied to the subject.
 */
class observer final
{
private:
    using subject_ptr_t = std::shared_ptr<detail::observable_node>;
    using node_t = detail::observer_node;

public:
    UREACT_MAKE_MOVABLE_ONLY( observer );

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
        m_subject->unregister_observer( m_node );
    }

    /*!
     * @brief Tests if this instance is linked to a node
     */
    UREACT_WARN_UNUSED_RESULT bool is_valid() const
    {
        return m_node != nullptr;
    }

private:
    observer( node_t* node, subject_ptr_t subject )
        : m_node( node )
        , m_subject( std::move( subject ) )
    {}

    template <typename in_f, typename S>
    friend auto detail::observe_signal_impl( const signal<S>& subject, in_f&& func ) -> observer;

    template <typename f_in_t, typename E, typename... deps_t>
    friend auto detail::observe_events_impl(
        const events<E>& subject, const signal_pack<deps_t...>& dep_pack, f_in_t&& func )
        -> observer;

    /// Owned by subject
    node_t* m_node = nullptr;

    /// While the observer handle exists, the subject is not destroyed
    subject_ptr_t m_subject = nullptr;
};

/*!
 * @brief Takes ownership of an observer and automatically detaches it on scope exit
 */
class scoped_observer final
{
public:
    UREACT_MAKE_MOVABLE_ONLY( scoped_observer );

    /*!
     * @brief is not intended to be default constructive
     */
    scoped_observer() = delete;

    /*!
     * @brief Constructs instance from observer
     */
    scoped_observer( observer&& observer ) // NOLINT no explicit by design
        : m_observer( std::move( observer ) )
    {}

    /*!
     * @brief Destructor
     */
    ~scoped_observer()
    {
        m_observer.detach();
    }

    /*!
     * @brief Tests if this instance is linked to a node
     */
    UREACT_WARN_UNUSED_RESULT bool is_valid() const // TODO: check in tests
    {
        return m_observer.is_valid();
    }

private:
    observer m_observer;
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

template <class F>
using add_observer_action_next_ret
    = add_default_return_value_wrapper<F, observer_action, observer_action::next>;

template <typename E, typename F, typename... args_t>
struct add_observer_range_wrapper
{
    add_observer_range_wrapper( const add_observer_range_wrapper& other ) = default;

    add_observer_range_wrapper(
        add_observer_range_wrapper&& other ) noexcept // TODO: check in tests
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
class signal_observer_node final : public observer_node
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

template <typename E, typename func_t, typename... dep_values_t>
class events_observer_node final : public observer_node
{
public:
    template <typename F>
    events_observer_node( context& context,
        const std::shared_ptr<event_stream_node<E>>& subject,
        F&& func,
        const std::shared_ptr<signal_node<dep_values_t>>&... deps )
        : events_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps... )
    {
        get_graph().on_node_attach( *this, *subject );
        ( get_graph().on_node_attach( *this, *deps ), ... );
    }

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
                detach_functor<events_observer_node, std::shared_ptr<signal_node<dep_values_t>>...>(
                    *this ),
                m_deps );

            m_subject.reset();
        }
    }
};

template <typename in_f, typename S>
auto observe_signal_impl( const signal<S>& subject, in_f&& func ) -> observer
{
    static_assert( std::is_invocable_v<in_f, S>,
        "Passed functor should be callable with S. See documentation for ureact::observe()" );

    using F = std::decay_t<in_f>;
    using R = std::invoke_result_t<in_f, S>;

    // If return value of passed function is void, add observer_action::next as
    // default return value.
    using node_t = std::conditional_t<std::is_same_v<void, R>,
        signal_observer_node<S, add_observer_action_next_ret<F>>,
        signal_observer_node<S, F>>;

    const auto& subject_ptr = get_node_ptr( subject );

    std::unique_ptr<observer_node> node(
        new node_t( subject.get_context(), subject_ptr, std::forward<in_f>( func ) ) );
    observer_node* raw_node_ptr = node.get();

    subject_ptr->register_observer( std::move( node ) );

    return observer( raw_node_ptr, subject_ptr );
}

template <typename f_in_t, typename E, typename... deps_t>
auto observe_events_impl(
    const events<E>& subject, const signal_pack<deps_t...>& dep_pack, f_in_t&& func ) -> observer
{
    using F = std::decay_t<f_in_t>;

    // clang-format off
    using wrapper_t =
        select_t<
            // observer_action func(event_range<E> range, const deps_t& ...)
            condition<std::is_invocable_r_v<observer_action, F, event_range<E>, deps_t...>,
                      F>,
            // observer_action func(const E&, const deps_t& ...)
            condition<std::is_invocable_r_v<observer_action, F, E, deps_t...>,
                      add_observer_range_wrapper<E, F, deps_t...>>,
            // void func(event_range<E> range, const deps_t& ...)
            condition<std::is_invocable_r_v<void, F, event_range<E>, deps_t...>,
                      add_observer_action_next_ret<F>>,
            // void func(const E&, const deps_t& ...)
            condition<std::is_invocable_r_v<void, F, E, deps_t...>,
                      add_observer_range_wrapper<E, add_observer_action_next_ret<F>, deps_t...>>,
            signature_mismatches>;
    // clang-format on

    static_assert( !std::is_same_v<wrapper_t, signature_mismatches>,
        "observe: Passed function does not match any of the supported signatures" );

    using node_t = events_observer_node<E, wrapper_t, deps_t...>;

    context& context = subject.get_context();

    auto node_builder = [&context, &subject, &func]( const signal<deps_t>&... deps ) {
        return new node_t( context,
            get_node_ptr( subject ),
            std::forward<f_in_t>( func ),
            get_node_ptr( deps )... );
    };

    const auto& subject_node = get_node_ptr( subject );

    std::unique_ptr<observer_node> node( std::apply( node_builder, dep_pack.data ) );

    observer_node* raw_node = node.get();

    subject_node->register_observer( std::move( node ) );

    return observer( raw_node, subject_node );
}

} // namespace detail

/*!
 * @brief Create observer for signal
 *
 *  When the signal value S of subject changes, func is called
 *
 *  The signature of func should be equivalent to:
 *  * void func(const S&)
 *  * observer_action func(const S&)
 *
 *  By returning observer_action::stop_and_detach, the observer function can request
 *  its own detachment. Returning observer_action::next keeps the observer attached.
 *  Using a void return type is the same as always returning observer_action::next.
 *
 *  @note Resulting observer can be ignored. Lifetime of observer node will match subject signal's lifetime
 */
template <typename F, typename S>
auto observe( const signal<S>& subject, F&& func ) -> observer
{
    return observe_signal_impl( subject, std::forward<F>( func ) );
}

/*!
 * @brief Create observer for temporary signal
 *
 *  Same as observe(const signal<S>& subject, F&& func),
 *  but subject signal is about to die so caller must use result, otherwise observation isn't performed.
 */
template <typename F, typename S>
UREACT_WARN_UNUSED_RESULT_MSG( "Observing the temporary so observer should be stored" )
auto observe( signal<S>&& subject, F&& func ) -> observer
{
    return observe_signal_impl( std::move( subject ), std::forward<F>( func ) );
}

/*!
 * @brief Create observer for event stream
 *
 *  For every event e in subject, func is called.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of func should be equivalent to:
 *  * observer_action func(event_range<E> range, const deps_t& ...)
 *  * observer_action func(const E&, const deps_t& ...)
 *  * void func(event_range<E> range, const deps_t& ...)
 *  * void func(const E&, const deps_t& ...)
 *
 *  By returning observer_action::stop_and_detach, the observer function can request
 *  its own detachment. Returning observer_action::next keeps the observer attached.
 *  Using a void return type is the same as always returning observer_action::next.
 *
 *  @note Resulting observer can be ignored. Lifetime of observer node will match subject signal's lifetime
 *  @note The event_range<E> option allows to explicitly batch process single turn events
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename F, typename E, typename... deps_t>
auto observe( const events<E>& subject, const signal_pack<deps_t...>& dep_pack, F&& func )
    -> observer
{
    return observe_events_impl( subject, dep_pack, std::forward<F>( func ) );
}

/*!
 * @brief Create observer for temporary event stream
 *
 *  Same as observe(const events<E>& subject, const signal_pack<deps_t...>& dep_pack, F&& func),
 *  but subject signal is about to die so caller must use result, otherwise observation isn't performed.
 */
template <typename F, typename E, typename... deps_t>
UREACT_WARN_UNUSED_RESULT_MSG( "Observing the temporary so observer should be stored" )
auto observe( events<E>&& subject, const signal_pack<deps_t...>& dep_pack, F&& func )
    -> observer // TODO: check in tests
{
    return observe_events_impl( std::move( subject ), dep_pack, std::forward<F>( func ) );
}

/*!
 * @brief Create observer for event stream
 *
 *  Version without synchronization with additional signals
 *
 *  See observe(const events<E>& subject, const signal_pack<deps_t...>& dep_pack, F&& func)
 */
template <typename F, typename E>
auto observe( const events<E>& subject, F&& func ) -> observer
{
    return observe_events_impl( subject, signal_pack<>(), std::forward<F>( func ) );
}

/*!
 * @brief Create observer for temporary event stream
 *
 *  Same as observe(const events<E>& subject, F&& func),
 *  but subject signal is about to die so caller must use result, otherwise observation isn't performed.
 */
template <typename F, typename E>
UREACT_WARN_UNUSED_RESULT_MSG( "Observing the temporary so observer should be stored" )
auto observe( events<E>&& subject, F&& func ) -> observer // TODO: check in tests
{
    return observe_events_impl( std::move( subject ), signal_pack<>(), std::forward<F>( func ) );
}

namespace detail
{

template <typename outer_t, typename inner_t>
class flatten_node final : public signal_node<inner_t>
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
class event_flatten_node final : public event_stream_node<inner_t>
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

/*!
 * @brief Create a new signal by flattening a signal of a signal
 */
template <typename inner_value_t>
UREACT_WARN_UNUSED_RESULT auto flatten( const signal<signal<inner_value_t>>& outer )
{
    context& context = outer.get_context();
    return signal<inner_value_t>(
        std::make_shared<detail::flatten_node<signal<inner_value_t>, inner_value_t>>(
            context, get_node_ptr( outer ), get_node_ptr( outer.get() ) ) );
}

/*!
 * @brief Create a new event stream by flattening a signal of an event stream
 */
template <typename inner_value_t>
UREACT_WARN_UNUSED_RESULT auto flatten( const signal<events<inner_value_t>>& outer )
{
    context& context = outer.get_context();
    return events<inner_value_t>(
        std::make_shared<detail::event_flatten_node<events<inner_value_t>, inner_value_t>>(
            context, get_node_ptr( outer ), get_node_ptr( outer.get() ) ) );
}

/*!
 * @brief Utility to flatten public signal attribute of class pointed be reference
 *
 *  For example we have a class Foo with a public signal bar: struct Foo{ signal<int> bar; };
 *  Also, we have signal that points to this class by reference: signal<Foo&> bar
 *  This utility receives a signal reference bar and attribute pointer &Foo::bar and flattens it to signal<int> foobar
 *
 *  @sa reactive_ptr does the same, but receives signal<Foo*>
 */
template <typename S, typename R, typename decayed_r = detail::decay_input_t<R>>
UREACT_WARN_UNUSED_RESULT auto reactive_ref(
    const signal<std::reference_wrapper<S>>& outer, R S::*attribute )
{
    return flatten( make_signal(
        outer, [attribute]( const S& s ) { return static_cast<decayed_r>( s.*attribute ); } ) );
}

/*!
 * @brief Utility to flatten public signal attribute of class pointed be pointer
 *
 *  For example we have a class Foo with a public signal bar: struct Foo{ signal<int> bar; };
 *  Also, we have signal that points to this class by pointer: signal<Foo*> bar
 *  This utility receives a signal reference bar and attribute pointer &Foo::bar and flattens it to signal<int> foobar
 *
 *  @sa reactive_ref does the same, but receives signal<Foo&>
 */
template <typename S, typename R, typename decayed_r = detail::decay_input_t<R>>
UREACT_WARN_UNUSED_RESULT auto reactive_ptr( const signal<S*>& outer, R S::*attribute )
{
    return flatten( make_signal(
        outer, [attribute]( const S* s ) { return static_cast<decayed_r>( s->*attribute ); } ) );
}

//==================================================================================================
// [[section]] Algorithms
//==================================================================================================

namespace detail
{

template <typename E, typename S, typename F, typename... args_t>
class add_fold_range_wrapper
{
public:
    template <typename f_in_t, class = disable_if_same_t<f_in_t, add_fold_range_wrapper>>
    explicit add_fold_range_wrapper( f_in_t&& func )
        : m_func( std::forward<f_in_t>( func ) )
    {}

    // TODO: possible optimization - move accum as much as possible. See std::accumulate
    S operator()( event_range<E> range, S accum, const args_t&... args )
    {
        for( const auto& e : range )
        {
            accum = m_func( e, accum, args... );
        }

        return accum;
    }

private:
    F m_func;
};

template <typename E, typename S, typename F, typename... args_t>
class add_fold_by_ref_range_wrapper
{
public:
    template <typename f_in_t, class = disable_if_same_t<f_in_t, add_fold_by_ref_range_wrapper>>
    explicit add_fold_by_ref_range_wrapper( f_in_t&& func )
        : m_func( std::forward<f_in_t>( func ) )
    {}

    void operator()( event_range<E> range, S& accum, const args_t&... args )
    {
        for( const auto& e : range )
        {
            m_func( e, accum, args... );
        }
    }

private:
    F m_func;
};

template <typename S, typename E, typename func_t, typename... dep_values_t>
class fold_node final : public signal_node<S>
{
public:
    template <typename T, typename F>
    fold_node( context& context,
        T&& init,
        const std::shared_ptr<event_stream_node<E>>& events,
        F&& func,
        const std::shared_ptr<signal_node<dep_values_t>>&... deps )
        : fold_node::signal_node( context, std::forward<T>( init ) )
        , m_events( events )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps... )
    {
        this->get_graph().on_node_attach( *this, *events );
        ( this->get_graph().on_node_attach( *this, *deps ), ... );
    }

    ~fold_node() override
    {
        this->get_graph().on_node_detach( *this, *m_events );

        apply( detach_functor<fold_node, std::shared_ptr<signal_node<dep_values_t>>...>( *this ),
            m_deps );
    }

    void tick( turn_type& turn ) override
    {
        m_events->set_current_turn( turn );

        bool changed = false;

        if( !m_events->events().empty() )
        {
            if constexpr( std::is_invocable_r_v<S, func_t, event_range<E>, S, dep_values_t...> )
            {
                S new_value = apply(
                    [this]( const std::shared_ptr<signal_node<dep_values_t>>&... args ) {
                        return m_func( event_range<E>( m_events->events() ),
                            this->m_value,
                            args->value_ref()... );
                    },
                    m_deps );

                if( !equals( new_value, this->m_value ) )
                {
                    changed = true;
                    this->m_value = std::move( new_value );
                }
            }
            else if constexpr(
                std::is_invocable_r_v<void, func_t, event_range<E>, S&, dep_values_t...> )
            {
                apply(
                    [this]( const std::shared_ptr<signal_node<dep_values_t>>&... args ) {
                        m_func( event_range<E>( m_events->events() ),
                            this->m_value,
                            args->value_ref()... );
                    },
                    m_deps );

                // Always assume change
                changed = true;
            }
            else
            {
                static_assert( always_false<S>, "Unsupported function signature" );
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

template <typename E, typename V, typename f_in_t, typename... deps_t, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto fold_impl(
    const events<E>& events, V&& init, const signal_pack<deps_t...>& dep_pack, f_in_t&& func )
    -> signal<S>
{
    using F = std::decay_t<f_in_t>;

    // clang-format off
    using node_t =
        select_t<
            // S func(const S&, event_range<E> range, const deps_t& ...)
            condition<std::is_invocable_r_v<S, F, event_range<E>, S, deps_t...>,
                                  fold_node<S, E, F, deps_t...>>,
            // S func(const S&, const E&, const deps_t& ...)
            condition<std::is_invocable_r_v<S, F, E, S, deps_t...>,
                                  fold_node<S, E, add_fold_range_wrapper<E, S, F, deps_t...>, deps_t...>>,
            // void func(S&, event_range<E> range, const deps_t& ...)
            condition<std::is_invocable_r_v<void, F, event_range<E>, S&, deps_t...>,
                                  fold_node<S, E, F, deps_t...>>,
            // void func(S&, const E&, const deps_t& ...)
            condition<std::is_invocable_r_v<void, F, E, S&, deps_t...>,
                                  fold_node<S, E, add_fold_by_ref_range_wrapper<E, S, F, deps_t...>, deps_t...>>,
            signature_mismatches>;
    // clang-format on

    static_assert( !std::is_same_v<node_t, signature_mismatches>,
        "fold: Passed function does not match any of the supported signatures" );

    context& context = events.get_context();

    auto node_builder = [&context, &events, &init, &func]( const signal<deps_t>&... deps ) {
        return signal<S>( std::make_shared<node_t>( context,
            std::forward<V>( init ),
            get_node_ptr( events ),
            std::forward<f_in_t>( func ),
            get_node_ptr( deps )... ) );
    };

    return std::apply( node_builder, dep_pack.data );
}

template <typename E>
class monitor_node final : public event_stream_node<E>
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

} // namespace detail

/*!
 * @brief Folds values from an event stream into a signal
 *
 *  Iteratively combines signal value with values from event stream.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of func should be equivalent to:
 *  * S func(const E& event, const S& accum, const deps_t& ...)
 *  * S func(event_range<E> range, const S& accum, const deps_t& ...)
 *  * void func(const E& event, S& accum, const deps_t& ...)
 *  * void func(event_range<E> range, S& accum, const deps_t& ...)
 *
 *  The fold parameters:
 *    [const events<E>& events, V&& init, const signal_pack<deps_t...>& dep_pack]
 *  match the corresponding arguments of the given function
 *    [const E& event_value, const S& accumulator, const deps_t& ...deps]
 *
 *  Creates a signal with an initial value v = init.
 *  * If the return type of func is S: For every received event e in events, v is updated to v = func(e,v, deps).
 *  * If the return type of func is void: For every received event e in events,
 *    v is passed by non-cost reference to func(v, e, deps), making it mutable.
 *    This variant can be used if copying and comparing S is prohibitively expensive.
 *    Because the old and new values cannot be compared, updates will always trigger a change.
 *
 *  @note order of arguments is inverse compared with std::accumulate() to correspond fold parameters
 *  @note The event_range<E> option allows to explicitly batch process single turn events
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename E, typename V, typename f_in_t, typename... deps_t, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto fold(
    const events<E>& events, V&& init, const signal_pack<deps_t...>& dep_pack, f_in_t&& func )
    -> signal<S>
{
    return fold_impl( events, std::forward<V>( init ), dep_pack, std::forward<f_in_t>( func ) );
}

/*!
 * @brief Folds values from an event stream into a signal
 *
 *  Version without synchronization with additional signals
 *
 *  See fold(const events<E>& events, V&& init, const signal_pack<deps_t...>& dep_pack, f_in_t&& func)
 */
template <typename E, typename V, typename f_in_t, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto fold( const events<E>& events, V&& init, f_in_t&& func ) -> signal<S>
{
    return fold_impl(
        events, std::forward<V>( init ), signal_pack<>(), std::forward<f_in_t>( func ) );
}

/*!
 * @brief Curried version of fold(const events<E>& events, V&& init, f_in_t&& func) algorithm used for "pipe" syntax
 */
template <typename V, typename f_in_t>
UREACT_WARN_UNUSED_RESULT auto fold( V&& init, f_in_t&& func )
{
    return closure{
        [init = std::forward<V>( init ), func = std::forward<f_in_t>( func )]( auto&& source ) {
            using arg_t = decltype( source );
            static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
            return fold( std::forward<arg_t>( source ), std::move( init ), func );
        } };
}

/*!
 * @brief Counts amount of received events into signal<S>
 *
 *  Type of resulting signal can be explicitly specified.
 *  Value type should be default constructing and prefix incremented
 *
 *  @warning Not to be confused with std::count(from, to, value)
 */
template <typename S = size_t, class E>
UREACT_WARN_UNUSED_RESULT auto count( const events<E>& source ) -> signal<S>
{
    return fold( source,
        S{},                       //
        []( const E&, S& accum ) { //
            ++accum;
        } );
}

/*!
 * @brief Curried version of count(const events<E>& source) algorithm used for "pipe" syntax
 */
template <typename S = size_t>
UREACT_WARN_UNUSED_RESULT auto count()
{
    return closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return count<S>( std::forward<arg_t>( source ) );
    } };
}

/*!
 * @brief Collects received events into signal<ContT<E>>
 *
 *  Type of resulting container must be specified explicitly, i.e. collect<std::vector>(src).
 *  Container type ContT should has either push_back(const E&) method or has insert(const E&) method.
 *  Mostly intended for testing purpose.
 *
 *  Semantically equivalent of ranges::to
 *
 *  @warning Use with caution, because there is no way to clear its value, or to ensure it destroyed
 *           because any observer or signal/events node will prolong its lifetime.
 */
template <template <typename...> class ContT, class E, class Cont = ContT<E>>
UREACT_WARN_UNUSED_RESULT auto collect( const ureact::events<E>& source ) -> signal<Cont>
{
    return fold( source,
        Cont{},                         //
        []( const E& e, Cont& accum ) { //
            if constexpr( detail::has_push_back_method_v<Cont, E> )
                accum.push_back( e );
            else if constexpr( detail::has_insert_method_v<Cont, E> )
                accum.insert( e );
            else
                static_assert( detail::always_false<Cont, E>, "Unsupported container" );
        } );
}

/*!
 * @brief Curried version of collect(const events<E>& source) algorithm used for "pipe" syntax
 */
template <template <typename...> class ContT>
UREACT_WARN_UNUSED_RESULT auto collect()
{
    return closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return collect<ContT>( std::forward<arg_t>( source ) );
    } };
}

/*!
 * @brief Holds the most recent event in a signal
 *
 *  Creates a @ref signal with an initial value v = init.
 *  For received event values e1, e2, ... eN in events, it is updated to v = eN.
 */
template <typename V, typename E>
UREACT_WARN_UNUSED_RESULT auto hold( const events<E>& source, V&& init ) -> signal<E>
{
    return fold( source,
        std::forward<V>( init ),                  //
        []( event_range<E> range, const auto& ) { //
            return *range.rbegin();
        } );
}

/*!
 * @brief Curried version of hold() algorithm used for "pipe" syntax
 */
template <typename V>
UREACT_WARN_UNUSED_RESULT auto hold( V&& init )
{
    return closure{ [init = std::forward<V>( init )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return hold( std::forward<arg_t>( source ), std::move( init ) );
    } };
}

/*!
 * @brief Sets the signal value to the value of a target signal when an event is received
 *
 *  Creates a signal with value v = target.get().
 *  The value is set on construction and updated only when receiving an event from trigger
 */
template <typename S, typename E>
UREACT_WARN_UNUSED_RESULT auto snapshot( const events<E>& trigger, const signal<S>& target )
    -> signal<S>
{
    return fold( trigger,
        target.get(),
        with( target ),
        []( event_range<E> range, const S&, const S& value ) { //
            return value;
        } );
}

/*!
 * @brief Curried version of snapshot() algorithm used for "pipe" syntax
 */
template <typename S>
UREACT_WARN_UNUSED_RESULT auto snapshot( const signal<S>& target )
{
    return closure{ [target = target]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return snapshot( std::forward<arg_t>( source ), target );
    } };
}

/*!
 * @brief Emits the value of a target signal when an event is received
 *
 *  Creates an event stream that emits target.get() when receiving an event from trigger.
 *  The values of the received events are irrelevant.
 */
template <typename S, typename E>
UREACT_WARN_UNUSED_RESULT auto pulse( const events<E>& trigger, const signal<S>& target )
    -> events<S>
{
    return process<S>( trigger,
        with( target ),
        []( event_range<E> range, event_emitter<S> out, const S& target_value ) {
            for( size_t i = 0, ie = range.size(); i < ie; ++i )
                out.emit( target_value );
        } );
}

/*!
 * @brief Curried version of pulse() algorithm used for "pipe" syntax
 */
template <typename S>
UREACT_WARN_UNUSED_RESULT auto pulse( const signal<S>& target )
{
    return closure{ [target = target]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return pulse( std::forward<arg_t>( source ), target );
    } };
}

/*!
 * @brief Emits value changes of signal as events
 *
 *  When target changes, emit the new value 'e = target.get()'.
 */
template <typename S>
UREACT_WARN_UNUSED_RESULT auto monitor( const signal<S>& target ) -> events<S>
{
    context& context = target.get_context();
    return events<S>(
        std::make_shared<detail::monitor_node<S>>( context, get_node_ptr( target ) ) );
}

/*!
 * @brief Curried version of monitor(const signal<S>& target) algorithm used for "pipe" syntax
 */
UREACT_WARN_UNUSED_RESULT inline auto monitor()
{
    return closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_signal_v<std::decay_t<arg_t>>, "Signal type is required" );
        return monitor( std::forward<arg_t>( source ) );
    } };
}

/*!
 * @brief Emits unit when target signal was changed
 *
 *  Creates a unit stream that emits when target is changed.
 */
template <typename S>
UREACT_WARN_UNUSED_RESULT auto changed( const signal<S>& target ) -> events<unit>
{
    return monitor( target ) | unify();
}

/*!
 * @brief Curried version of changed(const signal<S>& target) algorithm used for "pipe" syntax
 */
UREACT_WARN_UNUSED_RESULT inline auto changed()
{
    return closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_signal_v<std::decay_t<arg_t>>, "Signal type is required" );
        return changed( std::forward<arg_t>( source ) );
    } };
}

/*!
 * @brief Emits unit when target signal was changed to value
 *  Creates a unit stream that emits when target is changed and 'target.get() == value'.
 *  V and S should be comparable with ==.
 */
template <typename V, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto changed_to( const signal<S>& target, V&& value ) -> events<unit>
{
    return monitor( target ) | filter( [=]( const S& v ) { return v == value; } ) | unify();
}

/*!
 * @brief Curried version of changed_to(const signal<S>& target, V&& value) algorithm used for "pipe" syntax
 */
template <typename V, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT inline auto changed_to( V&& value )
{
    return closure{ [value = std::forward<V>( value )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_signal_v<std::decay_t<arg_t>>, "Signal type is required" );
        return changed_to( std::forward<arg_t>( source ), std::move( value ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_UREACT_H_
