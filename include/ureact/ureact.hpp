// ureact - C++ header-only FRP library
//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
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

#ifndef UREACT_UREACT_HPP
#define UREACT_UREACT_HPP

#define UREACT_VERSION_MAJOR 0
#define UREACT_VERSION_MINOR 8
#define UREACT_VERSION_PATCH 0
#define UREACT_VERSION_STR "0.8.0 wip"

#define UREACT_VERSION                                                                             \
    ( UREACT_VERSION_MAJOR * 10000 + UREACT_VERSION_MINOR * 100 + UREACT_VERSION_PATCH )

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#ifdef UREACT_USE_STD_ALGORITHM
#    include <algorithm>
#endif

// Preprocessor feature detections
// Mostly based on https://github.com/fmtlib/fmt/blob/master/include/fmt/core.h
// and on https://github.com/nemequ/hedley

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

#define UREACT_SETUP_COPY( ClassName, Action )                                                     \
    ClassName( const ClassName& ) = Action;                                                        \
    ClassName& operator=( const ClassName& ) = Action

#define UREACT_SETUP_MOVE( ClassName, Action )                                                     \
    ClassName( ClassName&& ) noexcept = Action;                                                    \
    ClassName& operator=( ClassName&& ) noexcept = Action

#define UREACT_MAKE_NONCOPYABLE( ClassName ) UREACT_SETUP_COPY( ClassName, delete )
#define UREACT_MAKE_COPYABLE( ClassName ) UREACT_SETUP_COPY( ClassName, default )
#define UREACT_MAKE_NONMOVABLE( ClassName ) UREACT_SETUP_MOVE( ClassName, delete )
#define UREACT_MAKE_MOVABLE( ClassName ) UREACT_SETUP_MOVE( ClassName, default )

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

class observer;

namespace detail
{

template <typename S>
class signal_node;

template <typename E>
class event_stream_node;

template <typename E, typename Op>
class event_op_node;

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
 * @brief Return if type is var_signal or its inheritor
 */
template <typename T>
struct is_var_signal : detail::is_base_of_template<var_signal, T>
{};

/*!
 * @brief Helper variable template for is_var_signal
 */
template <typename T>
inline constexpr bool is_var_signal_v = is_var_signal<T>::value;

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
 * @brief Return if type is signal's inheritor or signal_pack
 */
template <typename T>
struct is_signal_or_pack : std::disjunction<is_signal<T>, is_signal_pack<T>>
{};

/*!
 * @brief Helper variable template for is_signal_or_pack
 */
template <typename T>
inline constexpr bool is_signal_or_pack_v = is_signal_or_pack<T>::value;

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
 * @brief Return if type is event_source or its inheritor
 */
template <typename T>
struct is_event_source : detail::is_base_of_template<event_source, T>
{};

/*!
 * @brief Helper variable template for is_event_source
 */
template <typename T>
inline constexpr bool is_event_source_v = is_event_source<T>::value;

/*!
 * @brief Return if type is observer
 */
template <typename T>
struct is_observer : std::is_same<T, observer>
{};

/*!
 * @brief Helper variable template for is_observer
 */
template <typename T>
inline constexpr bool is_observer_v = is_observer<T>::value;

/*!
 * @brief Return if type is signal or event inheritor
 */
template <typename T>
struct is_observable : std::disjunction<is_signal<T>, is_event<T>>
{};

/*!
 * @brief Helper variable template for is_observable
 */
template <typename T>
inline constexpr bool is_observable_v = is_observable<T>::value;

/*!
 * @brief Return if type is signal or event or observer
 */
template <typename T>
struct is_reactive : std::disjunction<is_observable<T>, is_observer<T>>
{};

/*!
 * @brief Helper variable template for is_reactive
 */
template <typename T>
inline constexpr bool is_reactive_v = is_reactive<T>::value;

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

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wfloat-equal"
#endif

/// c++17 analog of equality_comparable concept from c++20
/// https://en.cppreference.com/w/cpp/concepts/equality_comparable
template <typename T, typename = void>
struct equality_comparable : std::false_type
{};

template <typename T>
struct equality_comparable<T, std::void_t<decltype( std::declval<T>() == std::declval<T>() )>>
    : std::true_type
{};

template <typename T>
inline constexpr bool equality_comparable_v = equality_comparable<T>::value;

/*!
 * @brief std::equal_to analog intended to prevent reaction of signals to setting the same value as before aka "calming"
 *
 *  Additionally:
 *  * it equally compares signal<S> and events<E> even if their operator== is overloaded
 *  * it equally compares reference wrappers because they can be used as S for signal<S> and their operator== does unexpected compare
 *  * it returns false if types are not equally comparable otherwise
 */
template <typename T>
UREACT_WARN_UNUSED_RESULT constexpr bool equal_to( const T& lhs, const T& rhs )
{
    if constexpr( equality_comparable_v<T> )
    {
        return lhs == rhs;
    }
    else
    {
        return false;
    }
}

template <typename S>
UREACT_WARN_UNUSED_RESULT constexpr bool equal_to( const signal<S>& lhs, const signal<S>& rhs )
{
    return lhs.equal_to( rhs );
}

template <typename E>
UREACT_WARN_UNUSED_RESULT constexpr bool equal_to( const events<E>& lhs, const events<E>& rhs )
{
    return lhs.equal_to( rhs );
}

template <typename T>
UREACT_WARN_UNUSED_RESULT constexpr bool equal_to(
    const std::reference_wrapper<T>& lhs, const std::reference_wrapper<T>& rhs )
{
    return equal_to( lhs.get(), rhs.get() );
}

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic pop
#endif

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

/// Utility class to defer self detach of observers
class deferred_observer_detacher
{
public:
    void queue_observer_for_detach( observer_interface& obs )
    {
        m_detached_observers.push_back( &obs );
    }

protected:
    void detach_queued_observers()
    {
        for( observer_interface* o : m_detached_observers )
        {
            o->unregister_self();
        }
        m_detached_observers.clear();
    }

    std::vector<observer_interface*> m_detached_observers;
};

#if !defined( NDEBUG )
/// Utility class to check if callbacks passed in lift(), process() etc
/// are used properly
class callback_sanitizer
{
public:
    /// Return if external callback is in progress
    [[nodiscard]] bool is_locked() const
    {
        return m_is_locked;
    }

    /// Marks begin of an external callback
    void begin_external_callback()
    {
        assert( !m_is_locked );
        m_is_locked = true;
    }

    /// Marks end of an external callback
    void end_external_callback()
    {
        assert( m_is_locked );
        m_is_locked = false;
    }

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

private:
    bool m_is_locked = false;
};

#    define UREACT_CALLBACK_GUARD( _SELF_ ) callback_sanitizer::guard _( _SELF_ )
#else
#    define UREACT_CALLBACK_GUARD( _SELF_ )                                                        \
        do                                                                                         \
        {                                                                                          \
        } while( false )
#endif

class react_graph
    : public deferred_observer_detacher
#if !defined( NDEBUG )
    , public callback_sanitizer
#endif
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
        for( input_node_interface* p : m_changed_inputs )
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
        for( reactive_node* cur_node : m_scheduled_nodes.next_values() )
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
    for( reactive_node* successor : node.successors )
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
    for( reactive_node* successor : node.successors )
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
    context_internals() = default;

    UREACT_WARN_UNUSED_RESULT react_graph& get_graph()
    {
        return *m_graph;
    }

    UREACT_WARN_UNUSED_RESULT const react_graph& get_graph() const
    {
        return *m_graph;
    }

private:
    // context_internals and context should be non-movable because
    // node_base contains reference to context, and it will break if context lose its graph
    UREACT_MAKE_NONMOVABLE( context_internals );

    std::unique_ptr<react_graph> m_graph = std::make_unique<react_graph>();
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
        return this == &rsh;
    }

    UREACT_WARN_UNUSED_RESULT bool operator!=( const context& rsh ) const
    {
        return !( *this == rsh );
    }

private:
    friend class detail::node_base;

    /// Returns internals. Not intended to use in user code
    UREACT_WARN_UNUSED_RESULT friend context_internals& _get_internals( context& ctx )
    {
        return ctx;
    }
};

namespace detail
{

class node_base : public reactive_node
{
public:
    explicit node_base( context& context )
        : m_context( context )
    {
        assert( !get_graph().is_locked() && "Can't create node from callback" );
    }

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
    UREACT_MAKE_NONCOPYABLE( node_base );

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

    UREACT_WARN_UNUSED_RESULT bool equal_to( const reactive_base& other ) const
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

    // Assign a new value and do pulse only if new value is different from the current one
    template <class T>
    void pulse_if_value_changed( T&& new_value )
    {
        if( !equal_to( this->m_value, new_value ) )
        {
            this->m_value = std::forward<T>( new_value );
            this->get_graph().on_node_pulse( *this );
        }
    }

    // Perform pulse after value modification was performed
    void pulse_after_modify()
    {
        this->get_graph().on_node_pulse( *this );
    }

protected:
    S m_value;
};

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
            std::invoke( func, this->m_value );

            m_is_input_modified = true;
        }
        // There's a new_value, modify new_value instead.
        // The modified new_value will be handled like before, i.e. it'll be compared to m_value
        // in apply_input
        else
        {
            std::invoke( func, m_new_value );
        }
    }

    bool apply_input( turn_type& ) override
    {
        if( m_is_input_added )
        {
            m_is_input_added = false;

            if( !equal_to( this->m_value, m_new_value ) )
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

template <typename S, typename Op>
class signal_op_node final : public signal_node<S>
{
public:
    template <typename... Args>
    explicit signal_op_node( context& context, Args&&... args )
        : signal_op_node::signal_node( context )
        , m_op( std::forward<Args>( args )... )
    {
        this->m_value = evaluate();

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
        this->pulse_if_value_changed( evaluate() );
    }

    UREACT_WARN_UNUSED_RESULT Op steal_op()
    {
        assert( !m_was_op_stolen && "Op was already stolen" );
        m_was_op_stolen = true;
        m_op.detach( *this );
        return std::move( m_op );
    }

    UREACT_WARN_UNUSED_RESULT bool was_op_stolen() const
    {
        return m_was_op_stolen;
    }

private:
    auto evaluate()
    {
        UREACT_CALLBACK_GUARD( this->get_graph() );
        return m_op.evaluate();
    }

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
        assert( !this->m_node->get_graph().is_locked() && "Can't read signal value from callback" );
        return this->m_node->value_ref();
    }

    template <typename T>
    void set_value( T&& new_value ) const
    {
        auto node_ptr = get_var_node();
        auto& graph_ref = node_ptr->get_graph();
        assert( !graph_ref.is_locked() && "Can't set signal value from callback" );

        graph_ref.push_input( node_ptr,
            [node_ptr, &new_value] { node_ptr->set_value( std::forward<T>( new_value ) ); } );
    }

    template <typename F>
    void modify_value( const F& func ) const
    {
        auto node_ptr = get_var_node();
        auto& graph_ref = node_ptr->get_graph();
        assert( !graph_ref.is_locked() && "Can't modify signal value from callback" );

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
class var_signal : public signal<S>
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
     * @brief Construct from the given node
     */
    explicit var_signal( std::shared_ptr<Node>&& node )
        : var_signal::signal( std::move( node ) )
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

    using signal<S>::operator();

    /*!
     * @brief Set new signal value
     *
     *  Function object version of set(const S& new_value)
     */
    void operator()( const S& new_value )
    {
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( new_value );
    }

    /*!
     * @brief Set new signal value
     *
     *  Function object version of set(S&& new_value)
     *
     *  Specialization of operator()(const S& new_value) for rvalue
     */
    void operator()( S&& new_value )
    {
        assert( this->is_valid() && "Can't set new value for var_signal not attached to a node" );
        this->set_value( std::move( new_value ) );
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
    UREACT_WARN_UNUSED_RESULT Op steal_op() &&
    {
        assert( this->m_node.use_count() == 1
                && "temp_signal's node should be uniquely owned, otherwise it is misused" );
        auto* node_ptr = static_cast<Node*>( this->m_node.get() );
        return node_ptr->steal_op();
    }

    /*!
     * @brief Checks if internal operator was already stolen
     */
    UREACT_WARN_UNUSED_RESULT bool was_op_stolen() const
    {
        auto* node_ptr = static_cast<Node*>( this->m_node.get() );
        return node_ptr->was_op_stolen();
    }
};

/*!
 * @brief Interface for signal<S> that allows construction and assigment only for Owner class
 * 
 * member_signal is intended to be used as type for public members, so everybody can freely
 * access its public interface while being restricted from reassignment that should be allowed
 * only for the Owner class
 */
template <typename Owner, typename S>
class member_signal : public signal<S>
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_signal
     */
    member_signal() = default;

    /*!
     * @brief Copy construct from the given signal
     */
    member_signal( const signal<S>& src ) // NOLINT(google-explicit-constructor)
        : member_signal::signal( src )
    {}

    /*!
     * @brief Move construct from the given signal
     */
    member_signal( signal<S>&& src ) noexcept // NOLINT(google-explicit-constructor)
        : member_signal::signal( std::move( src ) )
    {}
};

/*!
 * @brief Interface for var_signal<S> that allows construction and assigment only for Owner class
 * 
 * member_var_signal is intended to be used as type for public members, so everybody can freely
 * access its public interface while being restricted from reassignment that should be allowed
 * only for the Owner class
 */
template <typename Owner, typename S>
class member_var_signal : public var_signal<S>
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_signal
     */
    member_var_signal() = default;

    /*!
     * @brief Copy construct from the given var_signal
     */
    member_var_signal( const var_signal<S>& src ) // NOLINT(google-explicit-constructor)
        : member_var_signal::var_signal( src )
    {}

    /*!
     * @brief Move construct from the given var_signal
     */
    member_var_signal( var_signal<S>&& src ) noexcept // NOLINT(google-explicit-constructor)
        : member_var_signal::var_signal( std::move( src ) )
    {}
};

/// Base class to setup aliases to member signal classes with specific owner class
template <class Owner>
class member_signal_user
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_signal_user
     */
    member_signal_user() = default;

    template <class S>
    using member_signal = member_signal<Owner, S>;

    template <class S>
    using member_var_signal = member_var_signal<Owner, S>;
};

/// Macro to setup aliases to member signal classes with specific owner class
#define UREACT_USE_MEMBER_SIGNALS( Owner )                                                         \
    template <class S>                                                                             \
    using member_signal = ::ureact::member_signal<Owner, S>;                                       \
    template <class S>                                                                             \
    using member_var_signal = ::ureact::member_var_signal<Owner, S>

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

template <typename S, typename V>
UREACT_WARN_UNUSED_RESULT auto make_var_signal( context& context, V&& v )
{
    return var_signal<S>{ std::make_shared<var_node<S>>( context, std::forward<V>( v ) ) };
}

template <typename V, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto make_var_impl( context& context, V&& v )
{
    // TODO: detect var_signal type and then construct it once
    //       unfortunately it is not directly possible with select_t and std::condition
    //       because all branches should be well-formed, that is not the case with
    //       attempt to access "typename S::value_t" from types without value_t member
    //       https://stackoverflow.com/questions/24098278/stdconditional-compile-time-branch-evaluation
    if constexpr( is_observable_v<S> )
    {
        // clang-format off
        using S2 =
            select_t<
                condition<is_signal_v<S>,   signal<typename S::value_t>>,
                condition<is_event_v<S>,    events<typename S::value_t>>,
                S>;
        // clang-format on

        return make_var_signal<S2>( context, std::forward<V>( v ) );
    }
    else
    {
        return make_var_signal<S>( context, std::forward<V>( v ) );
    }
}

} // namespace detail

/*!
 * @brief Create a new input signal node and links it to the returned var_signal instance
 */
template <typename V>
UREACT_WARN_UNUSED_RESULT auto make_var( context& context, V&& value )
{
    assert( !_get_internals( context ).get_graph().is_locked() && "Can't make var from callback" );
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
    assert(
        !_get_internals( context ).get_graph().is_locked() && "Can't make const from callback" );
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

UREACT_END_NAMESPACE

#endif // UREACT_UREACT_HPP
