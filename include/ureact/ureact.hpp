// ureact.hpp - C++ single-header FRP library
//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2022 Krylov Yaroslav.
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
#define UREACT_VERSION_MINOR 6
#define UREACT_VERSION_PATCH 0
#define UREACT_VERSION_STR "0.6.0"

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

template <typename S, typename Op>
class temp_signal;

template <typename E>
class events;

template <typename E>
class event_source;

template <typename... Values>
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
template <template <typename...> class Base, typename Derived>
struct is_base_of_template_impl
{
    template <typename... Ts>
    static constexpr std::true_type test( const Base<Ts...>* )
    {
        return {};
    }
    static constexpr std::false_type test( ... )
    {
        return {};
    }
    using type = decltype( test( std::declval<Derived*>() ) );
};

template <template <typename...> class Base, typename Derived>
using is_base_of_template = typename is_base_of_template_impl<Base, Derived>::type;

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
 * @brief Return if type is signal_pack
 */
template <typename T>
struct is_signal_pack : detail::is_base_of_template<signal_pack, T>
{};

/*!
 * @brief Helper variable template for is_signal_pack
 */
template <typename T>
inline constexpr bool is_signal_pack_v = is_signal_pack<T>::value;

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
template <typename ForwardIt, typename Value>
inline ForwardIt find( ForwardIt first, ForwardIt last, const Value& val )
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
template <typename ForwardIt, typename Pred>
ForwardIt find_if_not( ForwardIt first, ForwardIt last, Pred pred )
{
    for( ; first != last; ++first )
    {
        if( !pred( *first ) )
        {
            return first;
        }
    }
    return last;
}

// Code based on possible implementation at
// https://en.cppreference.com/w/cpp/algorithm/iter_swap
template <typename LhsForwardIt, typename RhsForwardIt>
void iter_swap( LhsForwardIt a, RhsForwardIt b )
{
    using std::swap;
    swap( *a, *b );
}

// Code based on possible implementation at
// https://en.cppreference.com/w/cpp/algorithm/partition
template <typename ForwardIt, typename Pred>
ForwardIt partition( ForwardIt first, ForwardIt last, Pred pred )
{
    first = detail::find_if_not( first, last, pred );
    if( first == last )
    {
        return first;
    }

    for( ForwardIt i = std::next( first ); i != last; ++i )
    {
        if( pred( *i ) )
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

template <typename L, typename R>
using disable_if_same_t = std::enable_if_t<!std::is_same_v<std::decay_t<L>, std::decay_t<R>>>;

template <typename L, typename R>
using is_same_decay = std::is_same<std::decay_t<L>, std::decay_t<R>>;

template <typename L, typename R>
inline constexpr bool is_same_decay_v = is_same_decay<L, R>::value;

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wfloat-equal"
#endif

template <typename L, typename R, typename = void>
struct equality_comparable_with : std::false_type
{};

template <typename L, typename R>
struct equality_comparable_with<L,
    R,
    std::void_t<decltype( std::declval<L>() == std::declval<R>() )>> : std::true_type
{};

template <typename L, typename R>
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

template <typename Node>
class node_vector
{
public:
    void add( Node& node )
    {
        m_data.push_back( &node );
    }

    void remove( const Node& node )
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
    std::vector<Node*> m_data;
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
    struct transaction_guard
    {
        react_graph& self;

        explicit transaction_guard( react_graph& self )
            : self( self )
        {
            ++self.m_transaction_level;
        }

        ~transaction_guard()
        {
            --self.m_transaction_level;

            if( self.m_transaction_level == 0 )
            {
                self.finalize_transaction();
            }
        }

        UREACT_MAKE_NONCOPYABLE( transaction_guard );
        UREACT_MAKE_NONMOVABLE( transaction_guard );
    };

    react_graph() = default;

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

private:
    std::unique_ptr<react_graph> m_graph;
};

// forward declaration
class node_base;

} // namespace detail

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

    /// Returns internals. Not intended to use in user code
    UREACT_WARN_UNUSED_RESULT friend context_internals& _get_internals( context& ctx )
    {
        return ctx;
    }
};

/*!
 * @brief Perform several changes atomically
 * @tparam F type of passed functor
 * @tparam Args types of additional arguments passed to functor F
 *
 *  Can pass additional arguments to the functiona and optionally return a result
 */
template <typename F,
    typename... Args,
    class = std::enable_if_t<std::is_invocable_v<F&&, Args&&...>>>
UREACT_WARN_UNUSED_RESULT auto do_transaction( context& ctx, F&& func, Args&&... args )
{
    auto& graph = _get_internals( ctx ).get_graph();

    detail::react_graph::transaction_guard _{ graph };

    if constexpr( std::is_same_v<std::invoke_result_t<F&&, Args&&...>, void> )
    {
        std::forward<F>( func )( std::forward<Args>( args )... );
    }
    else
    {
        return std::forward<F>( func )( std::forward<Args>( args )... );
    }
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

    template <typename Arg, class = std::enable_if_t<std::is_invocable_v<F, Arg>>>
    UREACT_WARN_UNUSED_RESULT auto operator()( Arg&& args ) const
    {
        return m_func( std::forward<Arg>( args ) );
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

template <typename Node, typename... Deps>
struct attach_functor
{
    explicit attach_functor( Node& node )
        : m_node( node )
    {}

    void operator()( const Deps&... deps ) const
    {
        ( attach( deps ), ... );
    }

    template <typename T>
    void attach( const T& op ) const
    {
        op.template attach_rec<Node>( *this );
    }

    template <typename T>
    void attach( const std::shared_ptr<T>& dep_ptr ) const
    {
        m_node.get_graph().on_node_attach( m_node, *dep_ptr );
    }

    Node& m_node;
};

template <typename Node, typename... Deps>
struct detach_functor
{
    explicit detach_functor( Node& node )
        : m_node( node )
    {}

    void operator()( const Deps&... deps ) const
    {
        ( detach( deps ), ... );
    }

    template <typename T>
    void detach( const T& op ) const
    {
        op.template detach_rec<Node>( *this );
    }

    template <typename T>
    void detach( const std::shared_ptr<T>& dep_ptr ) const
    {
        m_node.get_graph().on_node_detach( m_node, *dep_ptr );
    }

    Node& m_node;
};

template <typename... Deps>
class reactive_op_base
{
public:
    using dep_holder_t = std::tuple<Deps...>;

    template <typename... Args>
    explicit reactive_op_base( dont_move, Args&&... args )
        : m_deps( std::forward<Args>( args )... )
    {}

    reactive_op_base( reactive_op_base&& ) noexcept = default;
    reactive_op_base& operator=( reactive_op_base&& ) noexcept = default;

    template <typename Node>
    void attach( Node& node ) const
    {
        std::apply( attach_functor<Node, Deps...>{ node }, m_deps );
    }

    template <typename Node>
    void detach( Node& node ) const
    {
        std::apply( detach_functor<Node, Deps...>{ node }, m_deps );
    }

    template <typename Node, typename Functor>
    void attach_rec( const Functor& functor ) const
    {
        // Same memory layout, different func
        std::apply( reinterpret_cast<const attach_functor<Node, Deps...>&>( functor ), m_deps );
    }

    template <typename Node, typename Functor>
    void detach_rec( const Functor& functor ) const
    {
        std::apply( reinterpret_cast<const detach_functor<Node, Deps...>&>( functor ), m_deps );
    }

protected:
    dep_holder_t m_deps;
};

template <typename Node>
class reactive_base
{
public:
    reactive_base() = default;

    explicit reactive_base( std::shared_ptr<Node>&& node )
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

    UREACT_WARN_UNUSED_RESULT const std::shared_ptr<Node>& get_node() const
    {
        return m_node;
    }

protected:
    std::shared_ptr<Node> m_node;
};

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

template <typename S, typename F, typename... Deps>
class function_op : public reactive_op_base<Deps...>
{
public:
    template <typename FIn, typename... Args>
    explicit function_op( FIn&& func, Args&&... args )
        : function_op::reactive_op_base( dont_move(), std::forward<Args>( args )... )
        , m_func( std::forward<FIn>( func ) )
    {}

    function_op( function_op&& ) noexcept = default;
    function_op& operator=( function_op&& ) noexcept = default;

    UREACT_WARN_UNUSED_RESULT S evaluate()
    {
        return std::apply( eval_functor( m_func ), this->m_deps );
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

template <typename S, typename Op>
class signal_op_node final : public signal_node<S>
{
public:
    template <typename... Args>
    explicit signal_op_node( context& context, Args&&... args )
        : signal_op_node::signal_node( context )
        , m_op( std::forward<Args>( args )... )
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

    UREACT_WARN_UNUSED_RESULT Op steal_op()
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
    Op m_op;
    bool m_was_op_stolen = false;
};

template <typename S>
class signal_base : public reactive_base<signal_node<S>>
{
public:
    signal_base() = default;

    template <typename Node>
    explicit signal_base( std::shared_ptr<Node>&& node )
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
    using Node = detail::signal_node<S>;

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
    explicit signal( std::shared_ptr<Node>&& node )
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
    using Node = detail::var_node<S>;

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
        : var_signal::signal( std::make_shared<Node>( context, value ) )
    {}

    /*!
     * @brief Construct a fully functional var signal
     *
     * @note replacing type of value with universal reference version prevents class template argument deduction
     */
    var_signal( context& context, S&& value )
        : var_signal::signal( std::make_shared<Node>( context, std::move( value ) ) )
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
    explicit temp_signal( context& context, Args&&... args )
        : temp_signal::signal( std::make_shared<Node>( context, std::forward<Args>( args )... ) )
    {}

    /*!
     * @brief Return internal operator, leaving node invalid
     */
    UREACT_WARN_UNUSED_RESULT Op steal_op()
    {
        auto* node_ptr = static_cast<Node*>( this->m_node.get() );
        return node_ptr->steal_op();
    }

    /*!
     * @brief Checks if internal operator was already stolen
     */
    UREACT_WARN_UNUSED_RESULT bool was_op_stolen() const // TODO: check in tests
    {
        auto* node_ptr = static_cast<Node*>( this->m_node.get() );
        return node_ptr->was_op_stolen();
    }
};

/*!
 * @brief A wrapper type for a tuple of signal references
 * @tparam Values types of signal values
 *
 *  Created with @ref with()
 */
template <typename... Values>
class signal_pack final
{
public:
    /*!
     * @brief Class to store signals instead of signal references
     */
    class stored
    {
    public:
        /*!
         * @brief Construct from signals
         */
        explicit stored( const signal<Values>&... deps )
            : data( std::tie( deps... ) )
        {}

        /*!
         * @brief The wrapped tuple
         */
        std::tuple<signal<Values>...> data;
    };

    /*!
     * @brief Construct from signals
     */
    explicit signal_pack( const signal<Values>&... deps )
        : data( std::tie( deps... ) )
    {}

    /*!
     * @brief Construct from stored signals
     */
    explicit signal_pack( const stored& value )
        : data( std::apply(
            []( const signal<Values>&... deps ) { return std::tie( deps... ); }, value.data ) )
    {}

    /*!
     * @brief Convert signal references to signals so they can be stored
     */
    UREACT_WARN_UNUSED_RESULT stored store() const
    {
        return std::apply(
            []( const signal<Values>&... deps ) { return stored{ deps... }; }, data );
    }

    /*!
     * @brief The wrapped tuple
     */
    std::tuple<const signal<Values>&...> data;
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
 * @brief Create a new input signal node and links it to the returned var_signal instance
 */
template <typename V>
UREACT_WARN_UNUSED_RESULT auto make_var( context& context, V&& value )
{
    return make_var_impl( context, std::forward<V>( value ) );
}

/*!
 * @brief Create a new signal node and links it to the returned signal instance
 *
 *  Returned value doesn't have input interface and can be used a placeholder where signal is required.
 *  Currently it's just cosmetic function that can be expressed as signal{ make_var(context, value) }
 *  but it can be optimized in future.
 */
template <typename V, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto make_const( context& context, V&& value ) -> signal<S>
{
    return make_var_impl( context, std::forward<V>( value ) );
}

/*!
 * @brief Utility function to create a signal_pack from given signals
 * @tparam Values types of signal values
 *
 *  Creates a signal_pack from the signals passed as deps.
 *  Semantically, this is equivalent to std::tie.
 */
template <typename... Values>
UREACT_WARN_UNUSED_RESULT auto with( const signal<Values>&... deps )
{
    return signal_pack<Values...>( deps... );
}

/*!
 * @brief Create a new signal node with value v = func(arg_pack.get(), ...).
 * @tparam Values types of signal values
 *
 * This value is set on construction and updated when any args have changed
 *
 *  The signature of func should be equivalent to:
 *  * S func(const Values& ...)
 */
template <typename... Values,
    typename InF,
    typename F = std::decay_t<InF>,
    typename S = std::invoke_result_t<F, Values...>,
    typename Op = detail::function_op<S, F, detail::signal_node_ptr_t<Values>...>>
UREACT_WARN_UNUSED_RESULT auto lift( const signal_pack<Values...>& arg_pack, InF&& func )
{
    context& context = std::get<0>( arg_pack.data ).get_context();

    auto node_builder = [&context, &func]( const signal<Values>&... args ) {
        return temp_signal<S, Op>{ context, std::forward<InF>( func ), args.get_node()... };
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
template <typename Value, typename InF>
UREACT_WARN_UNUSED_RESULT auto lift( const signal<Value>& arg, InF&& func )
{
    return lift( with( arg ), std::forward<InF>( func ) );
}

/*!
 * @brief Curried version of lift(const signal_pack<Values...>& arg_pack, InF&& func) algorithm used for "pipe" syntax
 */
template <typename InF>
UREACT_WARN_UNUSED_RESULT inline auto lift( InF&& func )
{
    return closure{ [func = std::forward<InF>( func )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert(
            std::disjunction_v<is_signal<std::decay_t<arg_t>>, is_signal_pack<std::decay_t<arg_t>>>,
            "Signal type or signal_pack is required" );
        return lift( std::forward<arg_t>( source ), func );
    } };
}

namespace detail
{

// Full analog of std::binder1st that removed in c++17
// See https://en.cppreference.com/w/cpp/utility/functional/binder12
template <template <typename, typename> class FunctorBinaryOp,
    typename FirstArgument,
    typename SecondArgument,
    typename Fn = FunctorBinaryOp<FirstArgument, SecondArgument>>
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
    FirstArgument m_first_argument;
};

// Full analog of std::binder2nd that removed in c++17
// See https://en.cppreference.com/w/cpp/utility/functional/binder12
template <template <typename, typename> class FunctorBinaryOp,
    typename FirstArgument,
    typename SecondArgument,
    typename Fn = FunctorBinaryOp<FirstArgument, SecondArgument>>
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
    SecondArgument m_second_argument;
};

template <template <typename> class FunctorOp,
    typename Signal,
    class = std::enable_if_t<is_signal_v<Signal>>>
auto unary_operator_impl( const Signal& arg )
{
    using val_t = typename Signal::value_t;
    using F = FunctorOp<val_t>;
    using S = std::invoke_result_t<F, val_t>;
    using Op = function_op<S, F, signal_node_ptr_t<val_t>>;
    return temp_signal<S, Op>{ arg.get_context(), F(), arg.get_node() };
}

template <template <typename> class FunctorOp, typename Val, typename OpIn>
auto unary_operator_impl( temp_signal<Val, OpIn>&& arg )
{
    using F = FunctorOp<Val>;
    using S = std::invoke_result_t<F, Val>;
    using Op = function_op<S, F, OpIn>;
    return temp_signal<S, Op>{ arg.get_context(), F(), arg.steal_op() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftSignal,
    typename RightSignal,
    class = std::enable_if_t<is_signal_v<LeftSignal>>,
    class = std::enable_if_t<is_signal_v<RightSignal>>>
auto binary_operator_impl( const LeftSignal& lhs, const RightSignal& rhs )
{
    using LeftVal = typename LeftSignal::value_t;
    using RightVal = typename RightSignal::value_t;
    using F = FunctorOp<LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal, RightVal>;
    using Op = function_op<S, F, signal_node_ptr_t<LeftVal>, signal_node_ptr_t<RightVal>>;

    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return temp_signal<S, Op>{ context, F(), lhs.get_node(), rhs.get_node() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftSignal,
    typename RightValIn,
    typename LeftVal = typename LeftSignal::value_t,
    typename RightVal = std::decay_t<RightValIn>,
    class = std::enable_if_t<is_signal_v<LeftSignal>>,
    class = std::enable_if_t<!is_signal_v<RightVal>>>
auto binary_operator_impl( const LeftSignal& lhs, RightValIn&& rhs )
{
    using F = binder2nd<FunctorOp, LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal>;
    using Op = function_op<S, F, signal_node_ptr_t<LeftVal>>;

    context& context = lhs.get_context();

    return temp_signal<S, Op>{ context, F( std::forward<RightValIn>( rhs ) ), lhs.get_node() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftValIn,
    typename RightSignal,
    typename LeftVal = std::decay_t<LeftValIn>,
    typename RightVal = typename RightSignal::value_t,
    class = std::enable_if_t<!is_signal_v<LeftVal>>,
    class = std::enable_if_t<is_signal_v<RightSignal>>>
auto binary_operator_impl( LeftValIn&& lhs, const RightSignal& rhs )
{
    using F = binder1st<FunctorOp, LeftVal, RightVal>;
    using S = std::invoke_result_t<F, RightVal>;
    using Op = function_op<S, F, signal_node_ptr_t<RightVal>>;

    context& context = rhs.get_context();

    return temp_signal<S, Op>{ context, F( std::forward<LeftValIn>( lhs ) ), rhs.get_node() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftVal,
    typename LeftOp,
    typename RightVal,
    typename RightOp>
auto binary_operator_impl(
    temp_signal<LeftVal, LeftOp>&& lhs, temp_signal<RightVal, RightOp>&& rhs )
{
    using F = FunctorOp<LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal, RightVal>;
    using Op = function_op<S, F, LeftOp, RightOp>;

    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return temp_signal<S, Op>{ context, F(), lhs.steal_op(), rhs.steal_op() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftVal,
    typename LeftOp,
    typename RightSignal,
    class = std::enable_if_t<is_signal_v<RightSignal>>>
auto binary_operator_impl( temp_signal<LeftVal, LeftOp>&& lhs, const RightSignal& rhs )
{
    using RightVal = typename RightSignal::value_t;
    using F = FunctorOp<LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal, RightVal>;
    using Op = function_op<S, F, LeftOp, signal_node_ptr_t<RightVal>>;

    context& context = rhs.get_context();

    return temp_signal<S, Op>{ context, F(), lhs.steal_op(), rhs.get_node() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftSignal,
    typename RightVal,
    typename RightOp,
    class = std::enable_if_t<is_signal_v<LeftSignal>>>
auto binary_operator_impl( const LeftSignal& lhs, temp_signal<RightVal, RightOp>&& rhs )
{
    using LeftVal = typename LeftSignal::value_t;
    using F = FunctorOp<LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal, RightVal>;
    using Op = function_op<S, F, signal_node_ptr_t<LeftVal>, RightOp>;

    context& context = lhs.get_context();

    return temp_signal<S, Op>{ context, F(), lhs.get_node(), rhs.steal_op() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftVal,
    typename LeftOp,
    typename RightValIn,
    typename RightVal = std::decay_t<RightValIn>,
    class = std::enable_if_t<!is_signal_v<RightVal>>>
auto binary_operator_impl( temp_signal<LeftVal, LeftOp>&& lhs, RightValIn&& rhs )
{
    using F = binder2nd<FunctorOp, LeftVal, RightVal>;
    using S = std::invoke_result_t<F, LeftVal>;
    using Op = function_op<S, F, LeftOp>;

    context& context = lhs.get_context();

    return temp_signal<S, Op>{ context, F( std::forward<RightValIn>( rhs ) ), lhs.steal_op() };
}

template <template <typename, typename> class FunctorOp,
    typename LeftValIn,
    typename RightVal,
    typename RightOp,
    typename LeftVal = std::decay_t<LeftValIn>,
    class = std::enable_if_t<!is_signal_v<LeftVal>>>
auto binary_operator_impl( LeftValIn&& lhs, temp_signal<RightVal, RightOp>&& rhs )
{
    using F = binder1st<FunctorOp, LeftVal, RightVal>;
    using S = std::invoke_result_t<F, RightVal>;
    using Op = function_op<S, F, RightOp>;

    context& context = rhs.get_context();

    return temp_signal<S, Op>{ context, F( std::forward<LeftValIn>( lhs ) ), rhs.steal_op() };
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
    template <typename Signal,                                                                     \
        template <typename> class FunctorOp = detail::op_functor_##name,                           \
        class = std::enable_if_t<is_signal_v<std::decay_t<Signal>>>>                               \
    UREACT_WARN_UNUSED_RESULT auto operator op( Signal&& arg )                                     \
        UREACT_FUNCTION_BODY( detail::unary_operator_impl<FunctorOp>( UREACT_FWD( arg ) ) )

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
    template <typename Lhs,                                                                        \
        typename Rhs,                                                                              \
        template <typename, typename> class FunctorOp = detail::op_functor_##name,                 \
        class = std::enable_if_t<                                                                  \
            std::disjunction_v<is_signal<std::decay_t<Lhs>>, is_signal<std::decay_t<Rhs>>>>>       \
    UREACT_WARN_UNUSED_RESULT auto operator op( Lhs&& lhs, Rhs&& rhs ) /*                */        \
        UREACT_FUNCTION_BODY(                                                                      \
            detail::binary_operator_impl<FunctorOp>( UREACT_FWD( lhs ), UREACT_FWD( rhs ) ) )

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
 *  See Regular Void https://open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0146r1.html#ThinkingAboutVoid
 */
struct unit
{
    constexpr unit() = default;
    constexpr unit( const unit& ) = default;
    constexpr unit& operator=( const unit& ) = default;

    // unit can be constructed from any value
    template <class T>
    explicit constexpr unit( T&& ) noexcept // NOLINT(bugprone-forwarding-reference-overload)
    {}
};

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

template <typename E, typename Op>
class event_op_node final : public event_stream_node<E>
{
public:
    template <typename... Args>
    explicit event_op_node( context& context, Args&&... args )
        : event_op_node::event_stream_node( context )
        , m_op( std::forward<Args>( args )... )
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

    UREACT_WARN_UNUSED_RESULT Op steal_op() // TODO: check in tests
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

    Op m_op;
    bool m_was_op_stolen = false;
};

template <typename E>
class event_stream_base : public reactive_base<event_stream_node<E>>
{
public:
    event_stream_base() = default;

    UREACT_MAKE_COPYABLE( event_stream_base );
    UREACT_MAKE_MOVABLE( event_stream_base );

    template <typename Node>
    explicit event_stream_base( std::shared_ptr<Node>&& node )
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
    using Node = detail::event_stream_node<E>;

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
    explicit events( std::shared_ptr<Node>&& node )
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
    using Node = detail::event_source_node<E>;

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
        : event_source::events( std::make_shared<Node>( context ) )
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

template <typename E, typename... Deps>
class event_merge_op : public reactive_op_base<Deps...>
{
public:
    template <typename... Args>
    explicit event_merge_op( Args&&... args )
        : event_merge_op::reactive_op_base( dont_move(), std::forward<Args>( args )... )
    {}

    event_merge_op( event_merge_op&& other ) noexcept // TODO: check in tests
        : event_merge_op::reactive_op_base( std::move( other ) )
    {}

    template <typename Collector>
    void collect( const turn_type& turn, const Collector& collector ) const
    {
        std::apply( collect_functor<Collector>( turn, collector ), this->m_deps );
    }

    template <typename Collector, typename Functor>
    void collect_rec( const Functor& functor ) const // TODO: check in tests
    {
        std::apply( reinterpret_cast<const collect_functor<Collector>&>( functor ), this->m_deps );
    }

private:
    template <typename Collector>
    struct collect_functor
    {
        collect_functor( const turn_type& turn, const Collector& collector )
            : m_turn( turn )
            , m_collector( collector )
        {}

        void operator()( const Deps&... deps ) const
        {
            ( collect( deps ), ... );
        }

        template <typename T>
        void collect( const T& op ) const // TODO: check in tests
        {
            op.template collect_rec<Collector>( *this );
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

        const turn_type& m_turn;
        const Collector& m_collector;
    };
};

template <typename InE, typename OutE, typename Func, typename... DepValues>
class event_processing_node final : public event_stream_node<OutE>
{
public:
    template <typename F>
    event_processing_node( context& context,
        const std::shared_ptr<event_stream_node<InE>>& source,
        F&& func,
        const std::shared_ptr<signal_node<DepValues>>&... deps )
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

        std::apply(
            detach_functor<event_processing_node, std::shared_ptr<signal_node<DepValues>>...>(
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
            std::apply(
                [this]( const std::shared_ptr<signal_node<DepValues>>&... args ) {
                    m_func( event_range<InE>( m_source->events() ),
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
    using dep_holder_t = std::tuple<std::shared_ptr<signal_node<DepValues>>...>;

    std::shared_ptr<event_stream_node<InE>> m_source;

    Func m_func;
    dep_holder_t m_deps;
};

template <typename... Values>
class event_zip_node final : public event_stream_node<std::tuple<Values...>>
{
public:
    explicit event_zip_node(
        context& context, const std::shared_ptr<event_stream_node<Values>>&... sources )
        : event_zip_node::event_stream_node( context )
        , m_slots( sources... )
    {
        ( this->get_graph().on_node_attach( *this, *sources ), ... );
    }

    ~event_zip_node() override
    {
        std::apply(
            [this]( slot<Values>&... slots ) {
                ( this->get_graph().on_node_detach( *this, *slots.source ), ... );
            },
            m_slots );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );

        {
            // Move events into buffers
            std::apply( [&turn]( slot<Values>&... slots ) { ( fetch_buffer( turn, slots ), ... ); },
                m_slots );

            while( true )
            {
                bool is_ready = true;

                // All slots ready?
                std::apply(
                    [&is_ready]( slot<Values>&... slots ) {
                        // Todo: combine return values instead
                        ( check_slot( slots, is_ready ), ... );
                    },
                    m_slots );

                if( !is_ready )
                {
                    break;
                }

                // Pop values from buffers and emit tuple
                std::apply(
                    [this]( slot<Values>&... slots ) {
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

    std::tuple<slot<Values>...> m_slots;
};

template <typename OutE, typename InE, typename Op, typename... DepValues>
UREACT_WARN_UNUSED_RESULT auto process_impl(
    const events<InE>& source, const signal_pack<DepValues...>& dep_pack, Op&& op ) -> events<OutE>
{
    using F = std::decay_t<Op>;

    context& context = source.get_context();

    auto node_builder = [&context, &source, &op]( const signal<DepValues>&... deps ) {
        return events<OutE>( std::make_shared<event_processing_node<InE, OutE, F, DepValues...>>(
            context, source.get_node(), std::forward<Op>( op ), deps.get_node()... ) );
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
 * @brief Create a new events node and links it to the returned events instance
 *
 *  Event value type E has to be specified explicitly. It would be unit if it is omitted.
 *
 *  Returned value doesn't have input interface and can be used a placeholder where events is required.
 *  Currently it's just cosmetic function that can be expressed as events<E>{ make_source<E>(context) }
 *  but it can be optimized in future.
 */
template <typename E = unit>
UREACT_WARN_UNUSED_RESULT auto make_never( context& context ) -> events<E>
{
    return event_source<E>{ context };
}

/*!
 * @brief Emit all events in source1, ... sources
 *
 *  @warning Not to be confused with std::merge() or ranges::merge()
 */
template <typename Source, typename... Sources, typename E = Source>
UREACT_WARN_UNUSED_RESULT auto merge(
    const events<Source>& source1, const events<Sources>&... sources ) -> events<E>
{
    static_assert( sizeof...( Sources ) >= 1, "merge: 2+ arguments are required" );

    using Op = detail::event_merge_op<E,
        detail::event_stream_node_ptr_t<Source>,
        detail::event_stream_node_ptr_t<Sources>...>;

    context& context = source1.get_context();
    return events<E>( std::make_shared<detail::event_op_node<E, Op>>(
        context, source1.get_node(), sources.get_node()... ) );
}

/*!
 * @brief Create a new event stream by batch processing events from other stream
 *
 *  op is called with all events range from source in current turn.
 *  New events are emitted through "out".
 *  Synchronized values of signals in dep_pack are passed to op as additional arguments.
 *
 *  The signature of op should be equivalent to:
 *  * bool op(event_range<in_t> range, event_emitter<out_t> out, const Deps& ...)
 *
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 *  @note The type of outgoing events T has to be specified explicitly, i.e. process<T>(src, with(deps), op)
 */
template <typename OutE, typename InE, typename Op, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto process(
    const events<InE>& source, const signal_pack<Deps...>& dep_pack, Op&& op ) -> events<OutE>
{
    return detail::process_impl<OutE>( source, dep_pack, std::forward<Op>( op ) );
}

/*!
 * @brief Curried version of process(const events<in_t>& source, Op&& op) algorithm used for "pipe" syntax
 */
template <typename OutE, typename Op, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto process( const signal_pack<Deps...>& dep_pack, Op&& op )
{
    return closure{ [deps = dep_pack.store(), op = std::forward<Op>( op )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return process<OutE>( std::forward<arg_t>( source ), signal_pack<Deps...>{ deps }, op );
    } };
}

/*!
 * @brief Create a new event stream by batch processing events from other stream
 *
 *  Version without synchronization with additional signals
 *
 *  See process(const events<InE>& source, const signal_pack<Deps...>& dep_pack, Op&& op)
 */
template <typename OutE, typename InE, typename Op>
UREACT_WARN_UNUSED_RESULT auto process( const events<InE>& source, Op&& op ) -> events<OutE>
{
    return detail::process_impl<OutE>( source, signal_pack<>(), std::forward<Op>( op ) );
}

/*!
 * @brief Curried version of process(const events<in_t>& source, Op&& op) algorithm used for "pipe" syntax
 */
template <typename OutE, typename Op>
UREACT_WARN_UNUSED_RESULT auto process( Op&& op )
{
    return closure{ [op = std::forward<Op>( op )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return process<OutE>( std::forward<arg_t>( source ), op );
    } };
}

/*!
 * @brief Create a new event stream that filters events from other stream
 *
 *  For every event e in source, emit e if pred(e, deps...) == true.
 *  Synchronized values of signals in dep_pack are passed to op as additional arguments.
 *
 *  The signature of pred should be equivalent to:
 *  * bool pred(const E&, const Deps& ...)
 *
 *  Semantically equivalent of ranges::filter
 *
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename E, typename Pred, typename... DepValues>
UREACT_WARN_UNUSED_RESULT auto filter(
    const events<E>& source, const signal_pack<DepValues...>& dep_pack, Pred&& pred ) -> events<E>
{
    using F = std::decay_t<Pred>;
    using result_t = std::invoke_result_t<F, E, DepValues...>;
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
 * @brief Curried version of filter(const events<E>& source, const signal_pack<DepValues...>& dep_pack, Pred&& pred) algorithm used for "pipe" syntax
 */
template <typename Pred, typename... DepValues>
UREACT_WARN_UNUSED_RESULT auto filter( const signal_pack<DepValues...>& dep_pack, Pred&& pred )
{
    return closure{ [deps = dep_pack.store(), pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return filter( std::forward<arg_t>( source ), signal_pack<DepValues...>{ deps }, pred );
    } };
}

/*!
 * @brief Create a new event stream that filters events from other stream
 *
 *  Version without synchronization with additional signals
 *
 *  See filter(const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred)
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
 *  * T func(const E&, const Deps& ...)
 *
 *  Semantically equivalent of ranges::transform
 *
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename InE,
    typename F,
    typename... Deps,
    typename OutE = std::invoke_result_t<F, InE, Deps...>>
UREACT_WARN_UNUSED_RESULT auto transform(
    const events<InE>& source, const signal_pack<Deps...>& dep_pack, F&& func ) -> events<OutE>
{
    return detail::process_impl<OutE>( source,
        dep_pack, //
        [func = std::forward<F>( func )](
            event_range<InE> range, event_emitter<OutE> out, const auto... deps ) mutable {
            for( const auto& e : range )
                out.emit( func( e, deps... ) );
        } );
}

/*!
 * @brief Curried version of transform(const events<InE>& source, const signal_pack<Deps...>& dep_pack, F&& func) algorithm used for "pipe" syntax
 */
template <typename F, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto transform( const signal_pack<Deps...>& dep_pack, F&& func )
{
    return closure{ [deps = dep_pack.store(), func = std::forward<F>( func )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return transform( std::forward<arg_t>( source ), signal_pack<Deps...>{ deps }, func );
    } };
}

/*!
 * @brief Create a new event stream that transforms events from other stream
 *
 *  Version without synchronization with additional signals
 *
 *  See transform(const events<in_t>& source, const signal_pack<Deps...>& dep_pack, F&& func)
 */
template <typename InE, typename F, typename OutE = std::invoke_result_t<F, InE>>
UREACT_WARN_UNUSED_RESULT auto transform( const events<InE>& source, F&& func ) -> events<OutE>
{
    return transform( source, signal_pack<>(), std::forward<F>( func ) );
}

/*!
 * @brief Curried version of transform(const events<InE>& source, F&& func) algorithm used for "pipe" syntax
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
 * @brief Create a new event stream that casts events from other stream using static_cast
 *
 *  For every event e in source, emit t = static_cast<OutE>(e).
 *
 *  Type of resulting signal have to be explicitly specified.
 */
template <typename OutE, typename InE>
UREACT_WARN_UNUSED_RESULT auto cast( const events<InE>& source ) -> events<OutE>
{
    return detail::process_impl<OutE>( source,
        signal_pack<>(), //
        []( event_range<InE> range, event_emitter<OutE> out ) mutable {
            for( const auto& e : range )
                out.emit( static_cast<OutE>( e ) );
        } );
}

/*!
 * @brief Curried version of cast(const events<InE>& source) algorithm used for "pipe" syntax
 */
template <typename OutE>
UREACT_WARN_UNUSED_RESULT auto cast()
{
    return closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return cast<OutE>( std::forward<arg_t>( source ) );
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
 * @brief Skips the first elements of the source stream that satisfy the predicate
 *
 *  Takes events beginning at the first for which the predicate returns false.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of pred should be equivalent to:
 *  * bool func(const E&, const Deps& ...)
 */
template <typename E, typename... Deps, typename Pred>
UREACT_WARN_UNUSED_RESULT auto drop_while(
    const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred )
{
    auto dropper_while = [passed = false, pred = std::forward<Pred>( pred )](
                             const auto& e, const auto... deps ) mutable {
        passed = passed || !pred( e, deps... );
        return passed;
    };

    return filter( source, dep_pack, dropper_while );
}

/*!
 * @brief Curried version of drop_while(const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred) algorithm used for "pipe" syntax
 */
template <typename... Deps, typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto drop_while(
    const signal_pack<Deps...>& dep_pack, Pred&& pred )
{
    return closure{ [deps = dep_pack.store(), pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return drop_while( std::forward<arg_t>( source ), signal_pack<Deps...>( deps ), pred );
    } };
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
 *  * bool func(const E&, const Deps& ...)
 */
template <typename E, typename... Deps, typename Pred>
UREACT_WARN_UNUSED_RESULT auto take_while(
    const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred )
{
    auto taker_while = [passed = true, pred = std::forward<Pred>( pred )](
                           const auto& e, const auto... deps ) mutable {
        passed = passed && pred( e, deps... );
        return passed;
    };

    return filter( source, dep_pack, taker_while );
}

/*!
 * @brief Curried version of take_while(const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred) algorithm used for "pipe" syntax
 */
template <typename... Deps, typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto take_while(
    const signal_pack<Deps...>& dep_pack, Pred&& pred )
{
    return closure{ [deps = dep_pack.store(), pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return take_while( std::forward<arg_t>( source ), signal_pack<Deps...>( deps ), pred );
    } };
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
template <typename Source, typename... Sources>
UREACT_WARN_UNUSED_RESULT auto zip( const events<Source>& source1,
    const events<Sources>&... sources ) -> events<std::tuple<Source, Sources...>>
{
    static_assert( sizeof...( Sources ) >= 1, "zip: 2+ arguments are required" );

    context& context = source1.get_context();
    return events<std::tuple<Source, Sources...>>(
        std::make_shared<detail::event_zip_node<Source, Sources...>>(
            context, source1.get_node(), sources.get_node()... ) );
}

/*!
 * @brief Utility function to transform any event stream into a unit stream
 *
 *  Emits a unit for any event that passes source
 */
template <typename E>
UREACT_WARN_UNUSED_RESULT auto unify( const events<E>& source )
{
    return cast<unit>( source );
}

/*!
 * @brief Curried version of unify(events_t&& source) algorithm used for "pipe" syntax
 */
UREACT_WARN_UNUSED_RESULT inline auto unify()
{
    return cast<unit>();
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

template <typename InF, typename S>
auto observe_signal_impl( const signal<S>& subject, InF&& func ) -> observer;

template <typename FIn, typename E, typename... Deps>
auto observe_events_impl(
    const events<E>& subject, const signal_pack<Deps...>& dep_pack, FIn&& func ) -> observer;

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
    using Node = detail::observer_node;

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
    observer( Node* node, subject_ptr_t subject )
        : m_node( node )
        , m_subject( std::move( subject ) )
    {}

    template <typename InF, typename S>
    friend auto detail::observe_signal_impl( const signal<S>& subject, InF&& func ) -> observer;

    template <typename FIn, typename E, typename... Deps>
    friend auto detail::observe_events_impl(
        const events<E>& subject, const signal_pack<Deps...>& dep_pack, FIn&& func ) -> observer;

    /// Owned by subject
    Node* m_node = nullptr;

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
template <typename F, typename Ret, Ret return_value>
class add_default_return_value_wrapper
{
public:
    template <typename InF,
        class = std::enable_if_t<!is_same_decay_v<InF, add_default_return_value_wrapper>>>
    explicit add_default_return_value_wrapper( InF&& func )
        : m_func( std::forward<InF>( func ) )
    {}

    template <typename... Args>
    UREACT_WARN_UNUSED_RESULT Ret operator()( Args&&... args )
    {
        m_func( std::forward<Args>( args )... );
        return return_value;
    }

private:
    F m_func;
};

template <class F>
using add_observer_action_next_ret
    = add_default_return_value_wrapper<F, observer_action, observer_action::next>;

template <typename E, typename F, typename... Args>
struct add_observer_range_wrapper
{
    add_observer_range_wrapper( const add_observer_range_wrapper& other ) = default;

    add_observer_range_wrapper(
        add_observer_range_wrapper&& other ) noexcept // TODO: check in tests
        : m_func( std::move( other.m_func ) )
    {}

    template <typename FIn, class = disable_if_same_t<FIn, add_observer_range_wrapper>>
    explicit add_observer_range_wrapper( FIn&& func )
        : m_func( std::forward<FIn>( func ) )
    {}

    // TODO: move 'typename... Args' here
    observer_action operator()( event_range<E> range, const Args&... args )
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

template <typename S, typename F>
class signal_observer_node final : public observer_node
{
public:
    template <typename InF>
    signal_observer_node(
        context& context, const std::shared_ptr<signal_node<S>>& subject, InF&& func )
        : signal_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<InF>( func ) )
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
    F m_func;
};

template <typename E, typename F, typename... DepValues>
class events_observer_node final : public observer_node
{
public:
    template <typename InF>
    events_observer_node( context& context,
        const std::shared_ptr<event_stream_node<E>>& subject,
        InF&& func,
        const std::shared_ptr<signal_node<DepValues>>&... deps )
        : events_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<InF>( func ) )
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
                    = std::apply(
                          [this, &p]( const std::shared_ptr<signal_node<DepValues>>&... args ) {
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
    using DepHolder = std::tuple<std::shared_ptr<signal_node<DepValues>>...>;

    std::weak_ptr<event_stream_node<E>> m_subject;

    F m_func;
    DepHolder m_deps;

    void detach_observer() override
    {
        if( auto p = m_subject.lock() )
        {
            get_graph().on_node_detach( *this, *p );

            std::apply(
                detach_functor<events_observer_node, std::shared_ptr<signal_node<DepValues>>...>(
                    *this ),
                m_deps );

            m_subject.reset();
        }
    }
};

template <typename InF, typename S>
auto observe_signal_impl( const signal<S>& subject, InF&& func ) -> observer
{
    static_assert( std::is_invocable_v<InF, S>,
        "Passed functor should be callable with S. See documentation for ureact::observe()" );

    using F = std::decay_t<InF>;
    using R = std::invoke_result_t<InF, S>;

    // If return value of passed function is void, add observer_action::next as
    // default return value.
    using Node = std::conditional_t<std::is_same_v<void, R>,
        signal_observer_node<S, add_observer_action_next_ret<F>>,
        signal_observer_node<S, F>>;

    const auto& subject_ptr = subject.get_node();

    std::unique_ptr<observer_node> node(
        new Node( subject.get_context(), subject_ptr, std::forward<InF>( func ) ) );
    observer_node* raw_node_ptr = node.get();

    subject_ptr->register_observer( std::move( node ) );

    return observer( raw_node_ptr, subject_ptr );
}

template <typename FIn, typename E, typename... Deps>
auto observe_events_impl(
    const events<E>& subject, const signal_pack<Deps...>& dep_pack, FIn&& func ) -> observer
{
    using F = std::decay_t<FIn>;

    // clang-format off
    using wrapper_t =
        select_t<
            // observer_action func(event_range<E> range, const Deps& ...)
            condition<std::is_invocable_r_v<observer_action, F, event_range<E>, Deps...>,
                      F>,
            // observer_action func(const E&, const Deps& ...)
            condition<std::is_invocable_r_v<observer_action, F, E, Deps...>,
                      add_observer_range_wrapper<E, F, Deps...>>,
            // void func(event_range<E> range, const Deps& ...)
            condition<std::is_invocable_r_v<void, F, event_range<E>, Deps...>,
                      add_observer_action_next_ret<F>>,
            // void func(const E&, const Deps& ...)
            condition<std::is_invocable_r_v<void, F, E, Deps...>,
                      add_observer_range_wrapper<E, add_observer_action_next_ret<F>, Deps...>>,
            signature_mismatches>;
    // clang-format on

    static_assert( !std::is_same_v<wrapper_t, signature_mismatches>,
        "observe: Passed function does not match any of the supported signatures" );

    using Node = events_observer_node<E, wrapper_t, Deps...>;

    context& context = subject.get_context();

    auto node_builder = [&context, &subject, &func]( const signal<Deps>&... deps ) {
        return new Node(
            context, subject.get_node(), std::forward<FIn>( func ), deps.get_node()... );
    };

    const auto& subject_node = subject.get_node();

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
 *  * observer_action func(event_range<E> range, const Deps& ...)
 *  * observer_action func(const E&, const Deps& ...)
 *  * void func(event_range<E> range, const Deps& ...)
 *  * void func(const E&, const Deps& ...)
 *
 *  By returning observer_action::stop_and_detach, the observer function can request
 *  its own detachment. Returning observer_action::next keeps the observer attached.
 *  Using a void return type is the same as always returning observer_action::next.
 *
 *  @note Resulting observer can be ignored. Lifetime of observer node will match subject signal's lifetime
 *  @note The event_range<E> option allows to explicitly batch process single turn events
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename F, typename E, typename... Deps>
auto observe( const events<E>& subject, const signal_pack<Deps...>& dep_pack, F&& func ) -> observer
{
    return observe_events_impl( subject, dep_pack, std::forward<F>( func ) );
}

/*!
 * @brief Create observer for temporary event stream
 *
 *  Same as observe(const events<E>& subject, const signal_pack<Deps...>& dep_pack, F&& func),
 *  but subject signal is about to die so caller must use result, otherwise observation isn't performed.
 */
template <typename F, typename E, typename... Deps>
UREACT_WARN_UNUSED_RESULT_MSG( "Observing the temporary so observer should be stored" )
auto observe( events<E>&& subject, const signal_pack<Deps...>& dep_pack, F&& func )
    -> observer // TODO: check in tests
{
    return observe_events_impl( std::move( subject ), dep_pack, std::forward<F>( func ) );
}

/*!
 * @brief Create observer for event stream
 *
 *  Version without synchronization with additional signals
 *
 *  See observe(const events<E>& subject, const signal_pack<Deps...>& dep_pack, F&& func)
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

template <typename OuterS, typename InnerS>
class signal_flatten_node final : public signal_node<InnerS>
{
public:
    signal_flatten_node( context& context,
        std::shared_ptr<signal_node<OuterS>> outer,
        const std::shared_ptr<signal_node<InnerS>>& inner )
        : signal_flatten_node::signal_node( context, inner->value_ref() )
        , m_outer( std::move( outer ) )
        , m_inner( inner )
    {
        this->get_graph().on_node_attach( *this, *m_outer );
        this->get_graph().on_node_attach( *this, *m_inner );
    }

    ~signal_flatten_node() override
    {
        this->get_graph().on_node_detach( *this, *m_inner );
        this->get_graph().on_node_detach( *this, *m_outer );
    }

    void tick( turn_type& ) override
    {
        const auto& new_inner = m_outer->value_ref().get_node();

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
    std::shared_ptr<signal_node<OuterS>> m_outer;
    std::shared_ptr<signal_node<InnerS>> m_inner;
};

template <typename OuterS, typename InnerE>
class event_flatten_node final : public event_stream_node<InnerE>
{
public:
    event_flatten_node( context& context,
        const std::shared_ptr<signal_node<OuterS>>& outer,
        const std::shared_ptr<event_stream_node<InnerE>>& inner )
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

        auto new_inner = m_outer->value_ref().get_node();

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
    std::shared_ptr<signal_node<OuterS>> m_outer;
    std::shared_ptr<event_stream_node<InnerE>> m_inner;
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
template <typename InnerS>
UREACT_WARN_UNUSED_RESULT auto flatten( const signal<signal<InnerS>>& outer ) -> signal<InnerS>
{
    context& context = outer.get_context();
    return signal<InnerS>{ std::make_shared<detail::signal_flatten_node<signal<InnerS>, InnerS>>(
        context, outer.get_node(), outer.get().get_node() ) };
}

/*!
 * @brief Create a new event stream by flattening a signal of an event stream
 */
template <typename InnerE>
UREACT_WARN_UNUSED_RESULT auto flatten( const signal<events<InnerE>>& outer ) -> events<InnerE>
{
    context& context = outer.get_context();
    return events<InnerE>{ std::make_shared<detail::event_flatten_node<events<InnerE>, InnerE>>(
        context, outer.get_node(), outer.get().get_node() ) };
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
template <typename S, typename R, typename DecayedR = detail::decay_input_t<R>>
UREACT_WARN_UNUSED_RESULT auto reactive_ref(
    const signal<std::reference_wrapper<S>>& outer, R S::*attribute )
{
    return flatten( lift(
        outer, [attribute]( const S& s ) { return static_cast<DecayedR>( s.*attribute ); } ) );
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
template <typename S, typename R, typename DecayedR = detail::decay_input_t<R>>
UREACT_WARN_UNUSED_RESULT auto reactive_ptr( const signal<S*>& outer, R S::*attribute )
{
    return flatten( lift(
        outer, [attribute]( const S* s ) { return static_cast<DecayedR>( s->*attribute ); } ) );
}

UREACT_END_NAMESPACE

#endif // UREACT_UREACT_H_
