// =============================================================
// == DO NOT MODIFY THIS FILE BY HAND - IT IS AUTO GENERATED! ==
// =============================================================
//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// ----------------------------------------------------------------
// Ureact v0.8.1
// Generated: 2023-02-11 11:51:45.202303
// ----------------------------------------------------------------
// ureact - C++ header-only FRP library
// The library is heavily influenced by cpp.react - https://github.com/snakster/cpp.react
// which uses the Boost Software License - Version 1.0
// see here - https://github.com/snakster/cpp.react/blob/master/LICENSE_1_0.txt
// Project started as a complex refactoring and transformation of cpp.react's codebase
// but with different design goals.
// The documentation can be found at the library's page:
// https://github.com/YarikTH/ureact/blob/main/README.md
// ----------------------------------------------------------------
// This file is an amalgamation of multiple different files.
// You probably shouldn't edit it directly.
// It is mostly intended to be used from godbolt and similar tools.
// ----------------------------------------------------------------
#ifndef UREACT_UREACT_AMALGAMATED_HPP
#define UREACT_UREACT_AMALGAMATED_HPP

#ifndef UREACT_CAST_HPP
#define UREACT_CAST_HPP


#ifndef UREACT_DETAIL_CLOSURE_HPP
#define UREACT_DETAIL_CLOSURE_HPP

#include <utility>


#ifndef UREACT_DETAIL_DEFINES_HPP
#define UREACT_DETAIL_DEFINES_HPP

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

#endif //UREACT_DETAIL_DEFINES_HPP

#ifndef UREACT_TYPE_TRAITS_HPP
#define UREACT_TYPE_TRAITS_HPP

#include <type_traits>


UREACT_BEGIN_NAMESPACE

namespace detail
{

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
 * @brief Helper class to mark failing of class match
 */
struct signature_mismatches;

/*!
 * @brief Helper for static assert
 */
template <typename...>
constexpr inline bool always_false = false;

/*!
 * @brief Helper to define emplace constructor that forwards args into child
 */
struct dont_move
{};

/*!
 * @brief Utility to help with variadic R-value emplace constructors
 */
template <typename L, typename R>
using disable_if_same_t = std::enable_if_t<!std::is_same_v<std::decay_t<L>, std::decay_t<R>>>;

} // namespace detail


// Forward
template <typename S>
class signal;

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



// Forward
template <typename S>
class var_signal;

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



// Forward
template <typename... Values>
class signal_pack;

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



// Forward
template <typename E>
class events;

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



// Forward
template <typename E>
class event_source;

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



// Forward
class observer;

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


UREACT_END_NAMESPACE

#endif //UREACT_TYPE_TRAITS_HPP

UREACT_BEGIN_NAMESPACE

namespace detail
{

// Forward
template <class F>
class closure;

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

    template <typename Arg, class = std::enable_if_t<std::is_invocable_v<F, Arg&&>>>
    UREACT_WARN_UNUSED_RESULT auto operator()( Arg&& arg ) const -> decltype( auto )
    {
        if constexpr( std::is_same_v<std::invoke_result_t<F, Arg&&>, void> )
        {
            m_func( std::forward<Arg>( arg ) );
        }
        else
        {
            return m_func( std::forward<Arg>( arg ) );
        }
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
UREACT_WARN_UNUSED_RESULT auto operator|( Arg&& arg, Closure&& closure_obj ) -> decltype( auto )
{
    if constexpr( is_closure_v<std::decay_t<Arg>> )
    {
        // chain two closures to make another one
        using FirstClosure = Arg;
        using SecondClosure = Closure;
        return detail::closure{
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

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_CLOSURE_HPP

#ifndef UREACT_TRANSFORM_HPP
#define UREACT_TRANSFORM_HPP


#ifndef UREACT_PROCESS_HPP
#define UREACT_PROCESS_HPP

#include <functional>


#ifndef UREACT_DETAIL_LINKER_FUNCTOR_HPP
#define UREACT_DETAIL_LINKER_FUNCTOR_HPP

#include <memory>


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
        this->m_graph.on_node_attach( this->m_node, *dep_ptr );
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
        this->m_graph.on_node_detach( this->m_node, *dep_ptr );
    }
};

} // namespace detail

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_LINKER_FUNCTOR_HPP

#ifndef UREACT_EVENT_EMITTER_HPP
#define UREACT_EVENT_EMITTER_HPP

#include <vector>


#ifndef UREACT_UNIT_HPP
#define UREACT_UNIT_HPP


UREACT_BEGIN_NAMESPACE

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

UREACT_END_NAMESPACE

#endif //UREACT_UNIT_HPP

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Represents output stream of events.
 *
 *  It is std::back_insert_iterator analog.
 *  Additionally to std::back_insert_iterator interface it provides operator<< overload
 */
template <typename E = unit>
class event_emitter final
{
public:
    using container_type = std::vector<E>;
    using iterator_category = std::output_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = E;
    using pointer = value_type*;
    using reference = value_type&;

    /*!
     * @brief Constructor
     */
    explicit event_emitter( container_type& container )
        : m_container( &container )
    {}

    // clang-format off
    event_emitter& operator*()       { return *this; }
    event_emitter& operator++()      { return *this; }
    event_emitter  operator++( int ) { return *this; } // NOLINT
    // clang-format on

    /*!
     * @brief Adds e to the queue of outgoing events
     */
    event_emitter& operator=( const E& e )
    {
        m_container->push_back( e );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     *
     * Specialization of operator=(const E& e) for rvalue
     */
    event_emitter& operator=( E&& e )
    {
        m_container->push_back( std::move( e ) );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events
     */
    event_emitter& operator<<( const E& e )
    {
        m_container->push_back( e );
        return *this;
    }

    /*!
     * @brief Adds e to the queue of outgoing events
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

UREACT_END_NAMESPACE

#endif //UREACT_EVENT_EMITTER_HPP

#ifndef UREACT_EVENT_RANGE_HPP
#define UREACT_EVENT_RANGE_HPP

#include <vector>


UREACT_BEGIN_NAMESPACE

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
    UREACT_WARN_UNUSED_RESULT const_reverse_iterator rend() const
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
    UREACT_WARN_UNUSED_RESULT bool empty() const
    {
        return m_data.empty();
    }

private:
    const std::vector<E>& m_data;
};

UREACT_END_NAMESPACE

#endif //UREACT_EVENT_RANGE_HPP

#ifndef UREACT_EVENTS_HPP
#define UREACT_EVENTS_HPP


#ifndef UREACT_CONTEXT_HPP
#define UREACT_CONTEXT_HPP


#ifndef UREACT_DETAIL_BASE_HPP
#define UREACT_DETAIL_BASE_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>


#ifdef UREACT_USE_STD_ALGORITHM
#    include <algorithm>
#endif

UREACT_BEGIN_NAMESPACE

class context;

class transaction;

namespace detail
{
class context_internals;
}

/// TODO: looks ugly. Replace context_internals with more proper feature
UREACT_WARN_UNUSED_RESULT detail::context_internals& _get_internals( context& ctx );

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

    UREACT_MAKE_NONCOPYABLE( observable );
    UREACT_MAKE_NONMOVABLE( observable );

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
    react_graph() = default;

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
    friend class ureact::transaction;

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

protected:
    void attach_to( reactive_node& parent )
    {
        get_graph().on_node_attach( *this, parent );
    }

    void detach_from( reactive_node& parent )
    {
        get_graph().on_node_detach( *this, parent );
    }

    void dynamic_attach_to( reactive_node& parent )
    {
        get_graph().on_dynamic_node_attach( *this, parent );
    }

    void dynamic_detach_from( reactive_node& parent )
    {
        get_graph().on_dynamic_node_detach( *this, parent );
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

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_BASE_HPP

UREACT_BEGIN_NAMESPACE

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

UREACT_END_NAMESPACE

#endif //UREACT_CONTEXT_HPP

UREACT_BEGIN_NAMESPACE

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

    void pulse_if_has_events()
    {
        if( !m_events.empty() )
        {
            this->get_graph().on_node_pulse( *this );
        }
    }

protected:
    data_t m_events;

private:
    unsigned m_cur_turn_id{ std::numeric_limits<unsigned>::max() };
};

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

        this->pulse_if_has_events();
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
        assert( !graph_ref.is_locked() && "Can't emit event from callback" );

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
class event_source : public events<E>
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
     * @brief Construct from the given node
     */
    explicit event_source( std::shared_ptr<Node>&& node )
        : event_source::events( std::move( node ) )
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

    // clang-format off
    iterator& operator*()       { return *this; }
    iterator& operator++()      { return *this; }
    iterator  operator++( int ) { return *this; } // NOLINT
    // clang-format on

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
 * @brief Interface for events<S> that allows construction and assigment only for Owner class
 * 
 * member_events is intended to be used as type for public members, so everybody can freely
 * access its public interface while being restricted from reassignment that should be allowed
 * only for the Owner class
 */
template <typename Owner, typename E>
class member_events : public events<E>
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_events
     */
    member_events() = default;

    /*!
     * @brief Copy construct from the given events
     */
    member_events( const events<E>& src ) // NOLINT(google-explicit-constructor)
        : member_events::events( src )
    {}

    /*!
     * @brief Move construct from the given events
     */
    member_events( events<E>&& src ) noexcept // NOLINT(google-explicit-constructor)
        : member_events::events( std::move( src ) )
    {}
};

/*!
 * @brief Interface for event_source<S> that allows construction and assigment only for Owner class
 * 
 * member_event_source is intended to be used as type for public members, so everybody can freely
 * access its public interface while being restricted from reassignment that should be allowed
 * only for the Owner class
 */
template <typename Owner, typename E>
class member_event_source : public event_source<E>
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_event_source
     */
    member_event_source() = default;

    /*!
     * @brief Copy construct from the given event_source
     */
    member_event_source( const event_source<E>& src ) // NOLINT(google-explicit-constructor)
        : member_event_source::event_source( src )
    {}

    /*!
     * @brief Move construct from the given event_source
     */
    member_event_source( event_source<E>&& src ) noexcept // NOLINT(google-explicit-constructor)
        : member_event_source::event_source( std::move( src ) )
    {}
};

/// Base class to setup aliases to member events classes with specific owner class
template <class Owner>
class member_events_user
{
    friend Owner;

    /*!
     * @brief Default construct @ref member_events_user
     */
    member_events_user() = default;

    template <class E>
    using member_events = member_events<Owner, E>;

    template <class E>
    using member_event_source = member_event_source<Owner, E>;
};

/// Macro to setup aliases to member events classes with specific owner class
#define UREACT_USE_MEMBER_EVENTS( Owner )                                                          \
    template <class E>                                                                             \
    using member_events = ::ureact::member_events<Owner, E>;                                       \
    template <class E>                                                                             \
    using member_event_source = ::ureact::member_event_source<Owner, E>

namespace detail
{

template <typename E>
UREACT_WARN_UNUSED_RESULT auto make_event_source( context& context )
{
    return event_source<E>{ std::make_shared<event_source_node<E>>( context ) };
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
    assert(
        !_get_internals( context ).get_graph().is_locked() && "Can't make source from callback" );
    return detail::make_event_source<E>( context );
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
    assert(
        !_get_internals( context ).get_graph().is_locked() && "Can't make never from callback" );
    return detail::make_event_source<E>( context );
}

UREACT_END_NAMESPACE

#endif //UREACT_EVENTS_HPP

#ifndef UREACT_SIGNAL_PACK_HPP
#define UREACT_SIGNAL_PACK_HPP

#include <tuple>


UREACT_BEGIN_NAMESPACE

template <typename S>
class signal;

/*!
 * @brief A wrapper type for a tuple of signals
 * @tparam Values types of signal values
 *
 *  Created with @ref with()
 */
template <typename... Values>
class signal_pack final
{
public:
    /*!
     * @brief Construct from signals
     */
    explicit signal_pack( const signal<Values>&... deps )
        : data( std::tie( deps... ) )
    {}

    /*!
     * @brief The wrapped tuple
     */
    std::tuple<signal<Values>...> data;
};

/*!
 * @brief Utility function to create a signal_pack from given signals
 * @tparam Values types of signal values
 *
 *  Creates a signal_pack from the signals passed as deps.
 *  Semantically, this is equivalent to std::make_tuple.
 *  
 *  TODO: receive universal references and make valid signal_pack from them
 */
template <typename... Values>
UREACT_WARN_UNUSED_RESULT auto with( const signal<Values>&... deps )
{
    return signal_pack<Values...>( deps... );
}

UREACT_END_NAMESPACE

#endif //UREACT_SIGNAL_PACK_HPP

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
class signal_node;

template <typename InE, typename OutE, typename Func, typename... Deps>
class event_processing_node final : public event_stream_node<OutE>
{
public:
    template <typename F>
    event_processing_node( context& context,
        const std::shared_ptr<event_stream_node<InE>>& source,
        F&& func,
        const std::shared_ptr<signal_node<Deps>>&... deps )
        : event_processing_node::event_stream_node( context )
        , m_source( source )
        , m_func( std::forward<F>( func ) )
        , m_deps( deps... )
    {
        this->attach_to( *source );
        ( this->attach_to( *deps ), ... );
    }

    ~event_processing_node() override
    {
        this->detach_from( *m_source );

        std::apply( detach_functor<event_processing_node>( *this ), m_deps );
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
                [this]( const std::shared_ptr<signal_node<Deps>>&... args ) {
                    UREACT_CALLBACK_GUARD( this->get_graph() );
                    std::invoke( m_func,
                        event_range<InE>( m_source->events() ),
                        event_emitter( this->m_events ),
                        args->value_ref()... );
                },
                m_deps );
        }

        this->pulse_if_has_events();
    }

private:
    using dep_holder_t = std::tuple<std::shared_ptr<signal_node<Deps>>...>;

    std::shared_ptr<event_stream_node<InE>> m_source;

    Func m_func;
    dep_holder_t m_deps;
};

template <typename OutE, typename InE, typename Op, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto process_impl(
    const events<InE>& source, const signal_pack<Deps...>& dep_pack, Op&& op ) -> events<OutE>
{
    using F = std::decay_t<Op>;

    context& context = source.get_context();

    auto node_builder = [&context, &source, &op]( const signal<Deps>&... deps ) {
        return events<OutE>( std::make_shared<event_processing_node<InE, OutE, F, Deps...>>(
            context, source.get_node(), std::forward<Op>( op ), deps.get_node()... ) );
    };

    return std::apply( node_builder, dep_pack.data );
}

} // namespace detail

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
 * @brief Curried version of process(const events<in_t>& source, Op&& op)
 */
template <typename OutE, typename Op, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto process( const signal_pack<Deps...>& dep_pack, Op&& op )
{
    return detail::closure{ [dep_pack = dep_pack, op = std::forward<Op>( op )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return process<OutE>( std::forward<arg_t>( source ), dep_pack, op );
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
 * @brief Curried version of process(const events<in_t>& source, Op&& op)
 */
template <typename OutE, typename Op>
UREACT_WARN_UNUSED_RESULT auto process( Op&& op )
{
    return detail::closure{ [op = std::forward<Op>( op )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return process<OutE>( std::forward<arg_t>( source ), op );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_PROCESS_HPP

UREACT_BEGIN_NAMESPACE

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
                out << std::invoke( func, e, deps... );
        } );
}

/*!
 * @brief Curried version of transform(const events<InE>& source, const signal_pack<Deps...>& dep_pack, F&& func)
 */
template <typename F, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto transform( const signal_pack<Deps...>& dep_pack, F&& func )
{
    return detail::closure{ [dep_pack = dep_pack, func = std::forward<F>( func )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return transform( std::forward<arg_t>( source ), dep_pack, func );
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
 * @brief Curried version of transform(const events<InE>& source, F&& func)
 */
template <typename F>
UREACT_WARN_UNUSED_RESULT auto transform( F&& func )
{
    return detail::closure{ [func = std::forward<F>( func )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return transform( std::forward<arg_t>( source ), func );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_TRANSFORM_HPP

UREACT_BEGIN_NAMESPACE

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
    return transform( source, //
        []( const InE& e ) {  //
            return static_cast<OutE>( e );
        } );
}

/*!
 * @brief Curried version of cast(const events<InE>& source)
 */
template <typename OutE>
UREACT_WARN_UNUSED_RESULT auto cast()
{
    return detail::closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return cast<OutE>( std::forward<arg_t>( source ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_CAST_HPP

#ifndef UREACT_CHANGED_HPP
#define UREACT_CHANGED_HPP


#ifndef UREACT_FILTER_HPP
#define UREACT_FILTER_HPP


UREACT_BEGIN_NAMESPACE

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
template <typename E, typename Pred, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto filter(
    const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred ) -> events<E>
{
    return detail::process_impl<E>( source,
        dep_pack, //
        [pred = std::forward<Pred>( pred )](
            event_range<E> range, event_emitter<E> out, const auto... deps ) mutable {
            for( const auto& e : range )
                if( std::invoke( pred, e, deps... ) )
                    out << e;
        } );
}

/*!
 * @brief Curried version of filter(const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred)
 */
template <typename Pred, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto filter( const signal_pack<Deps...>& dep_pack, Pred&& pred )
{
    return detail::closure{
        [dep_pack = dep_pack, pred = std::forward<Pred>( pred )]( auto&& source ) {
            using arg_t = decltype( source );
            static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
            return filter( std::forward<arg_t>( source ), dep_pack, pred );
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
 * @brief Curried version of filter(const events<E>& source, Pred&& pred)
 */
template <typename Pred>
UREACT_WARN_UNUSED_RESULT auto filter( Pred&& pred )
{
    return detail::closure{ [pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return filter( std::forward<arg_t>( source ), pred );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_FILTER_HPP

#ifndef UREACT_MONITOR_HPP
#define UREACT_MONITOR_HPP


UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
class signal_node;

template <typename E>
class monitor_node final : public event_stream_node<E>
{
public:
    monitor_node( context& context, const std::shared_ptr<signal_node<E>>& target )
        : monitor_node::event_stream_node( context )
        , m_target( target )
    {
        this->attach_to( *m_target );
    }

    ~monitor_node() override
    {
        this->detach_from( *m_target );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );

        this->m_events.push_back( m_target->value_ref() );

        this->pulse_if_has_events();
    }

private:
    const std::shared_ptr<signal_node<E>> m_target;
};

} // namespace detail

/*!
 * @brief Emits value changes of signal as events
 *
 *  When target changes, emit the new value 'e = target.get()'.
 */
template <typename S>
UREACT_WARN_UNUSED_RESULT auto monitor( const signal<S>& target ) -> events<S>
{
    context& context = target.get_context();
    return events<S>( std::make_shared<detail::monitor_node<S>>( context, target.get_node() ) );
}

/*!
 * @brief Curried version of monitor(const signal<S>& target)
 */
UREACT_WARN_UNUSED_RESULT inline auto monitor()
{
    return detail::closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_signal_v<std::decay_t<arg_t>>, "Signal type is required" );
        return monitor( std::forward<arg_t>( source ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_MONITOR_HPP

#ifndef UREACT_UNIFY_HPP
#define UREACT_UNIFY_HPP


UREACT_BEGIN_NAMESPACE

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
 * @brief Curried version of unify(events_t&& source)
 */
UREACT_WARN_UNUSED_RESULT inline auto unify()
{
    return cast<unit>();
}

UREACT_END_NAMESPACE

#endif // UREACT_UNIFY_HPP

UREACT_BEGIN_NAMESPACE

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
 * @brief Curried version of changed(const signal<S>& target)
 */
UREACT_WARN_UNUSED_RESULT inline auto changed()
{
    return detail::closure{ []( auto&& source ) {
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
 * @brief Curried version of changed_to(const signal<S>& target, V&& value)
 */
template <typename V, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT inline auto changed_to( V&& value )
{
    return detail::closure{ [value = std::forward<V>( value )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_signal_v<std::decay_t<arg_t>>, "Signal type is required" );
        return changed_to( std::forward<arg_t>( source ), std::move( value ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_CHANGED_HPP

#ifndef UREACT_COLLECT_HPP
#define UREACT_COLLECT_HPP


#ifndef UREACT_FOLD_HPP
#define UREACT_FOLD_HPP


#ifndef UREACT_SIGNAL_HPP
#define UREACT_SIGNAL_HPP


#ifndef UREACT_EQUAL_TO_HPP
#define UREACT_EQUAL_TO_HPP

#include <functional>
#include <type_traits>


UREACT_BEGIN_NAMESPACE

template <typename S>
class signal;

template <typename E>
class events;

namespace detail
{

template <typename Node>
class reactive_base;

/// c++17 analog of equality_comparable concept from c++20
/// https://en.cppreference.com/w/cpp/concepts/equality_comparable
template <typename T, typename = void>
struct equality_comparable : std::false_type
{};

// TODO: check if result of == is exactly bool
template <typename T>
struct equality_comparable<T, std::void_t<decltype( std::declval<T>() == std::declval<T>() )>>
    : std::true_type
{};

template <typename T>
inline constexpr bool equality_comparable_v = equality_comparable<T>::value;

} // namespace detail

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wfloat-equal"
#endif

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
    if constexpr( detail::equality_comparable_v<T> )
    {
        return lhs == rhs;
    }
    else
    {
        return false;
    }
}

// TODO: check if lhs.equal_to( rhs ) can be called instead of checking for specific types
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
UREACT_WARN_UNUSED_RESULT constexpr bool equal_to( //
    const std::reference_wrapper<T>& lhs,          //
    const std::reference_wrapper<T>& rhs )
{
    return equal_to( lhs.get(), rhs.get() );
}

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic pop
#endif

UREACT_END_NAMESPACE

#endif //UREACT_EQUAL_TO_HPP

UREACT_BEGIN_NAMESPACE

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

UREACT_END_NAMESPACE

#endif //UREACT_SIGNAL_HPP

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename E>
class event_stream_node;

template <typename E, typename S, typename F, typename... Args>
class add_fold_range_wrapper
{
public:
    template <typename InF, class = disable_if_same_t<InF, add_fold_range_wrapper>>
    explicit add_fold_range_wrapper( InF&& func )
        : m_func( std::forward<InF>( func ) )
    {}

    // TODO: possible optimization - move accum as much as possible. See std::accumulate
    // TODO: move 'typename... Args' here
    S operator()( event_range<E> range, S accum, const Args&... args )
    {
        for( const auto& e : range )
        {
            accum = std::invoke( m_func, e, accum, args... );
        }

        return accum;
    }

private:
    F m_func;
};

template <typename E, typename S, typename F, typename... Args>
class add_fold_by_ref_range_wrapper
{
public:
    template <typename InF, class = disable_if_same_t<InF, add_fold_by_ref_range_wrapper>>
    explicit add_fold_by_ref_range_wrapper( InF&& func )
        : m_func( std::forward<InF>( func ) )
    {}

    // TODO: move 'typename... Args' here
    void operator()( event_range<E> range, S& accum, const Args&... args )
    {
        for( const auto& e : range )
        {
            std::invoke( m_func, e, accum, args... );
        }
    }

private:
    F m_func;
};

template <typename S, typename E, typename F, typename... Deps>
class fold_node final : public signal_node<S>
{
public:
    template <typename InS, typename InF>
    fold_node( context& context,
        InS&& init,
        const std::shared_ptr<event_stream_node<E>>& events,
        InF&& func,
        const std::shared_ptr<signal_node<Deps>>&... deps )
        : fold_node::signal_node( context, std::forward<InS>( init ) )
        , m_events( events )
        , m_func( std::forward<InF>( func ) )
        , m_deps( deps... )
    {
        this->attach_to( *events );
        ( this->attach_to( *deps ), ... );
    }

    ~fold_node() override
    {
        this->detach_from( *m_events );

        std::apply( detach_functor<fold_node>( *this ), m_deps );
    }

    void tick( turn_type& turn ) override
    {
        m_events->set_current_turn( turn );

        if( m_events->events().empty() )
            return;

        if constexpr( std::is_invocable_r_v<S, F, event_range<E>, S, Deps...> )
        {
            this->pulse_if_value_changed( std::apply(
                [this]( const std::shared_ptr<signal_node<Deps>>&... args ) {
                    UREACT_CALLBACK_GUARD( this->get_graph() );
                    return std::invoke( m_func,
                        event_range<E>( m_events->events() ),
                        this->m_value,
                        args->value_ref()... );
                },
                m_deps ) );
        }
        else if constexpr( std::is_invocable_r_v<void, F, event_range<E>, S&, Deps...> )
        {
            std::apply(
                [this]( const std::shared_ptr<signal_node<Deps>>&... args ) {
                    UREACT_CALLBACK_GUARD( this->get_graph() );
                    std::invoke( m_func,
                        event_range<E>( m_events->events() ),
                        this->m_value,
                        args->value_ref()... );
                },
                m_deps );

            // Always assume change
            this->pulse_after_modify();
        }
        else
        {
            static_assert( always_false<S>, "Unsupported function signature" );
        }
    }

private:
    using DepHolder = std::tuple<std::shared_ptr<signal_node<Deps>>...>;

    std::shared_ptr<event_stream_node<E>> m_events;

    F m_func;
    DepHolder m_deps;
};

template <typename E, typename V, typename InF, typename... Deps, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto fold_impl(
    const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack, InF&& func )
    -> signal<S>
{
    using F = std::decay_t<InF>;

    // clang-format off
    using Node =
        select_t<
            // S func(const S&, event_range<E> range, const Deps& ...)
            condition<std::is_invocable_r_v<S, F, event_range<E>, S, Deps...>,
                                  fold_node<S, E, F, Deps...>>,
            // S func(const S&, const E&, const Deps& ...)
            condition<std::is_invocable_r_v<S, F, E, S, Deps...>,
                                  fold_node<S, E, add_fold_range_wrapper<E, S, F, Deps...>, Deps...>>,
            // void func(S&, event_range<E> range, const Deps& ...)
            condition<std::is_invocable_r_v<void, F, event_range<E>, S&, Deps...>,
                                  fold_node<S, E, F, Deps...>>,
            // void func(S&, const E&, const Deps& ...)
            condition<std::is_invocable_r_v<void, F, E, S&, Deps...>,
                                  fold_node<S, E, add_fold_by_ref_range_wrapper<E, S, F, Deps...>, Deps...>>,
            signature_mismatches>;
    // clang-format on

    static_assert( !std::is_same_v<Node, signature_mismatches>,
        "fold: Passed function does not match any of the supported signatures" );

    context& context = events.get_context();

    auto node_builder = [&context, &events, &init, &func]( const signal<Deps>&... deps ) {
        return signal<S>( std::make_shared<Node>( context,
            std::forward<V>( init ),
            events.get_node(),
            std::forward<InF>( func ),
            deps.get_node()... ) );
    };

    return std::apply( node_builder, dep_pack.data );
}

} // namespace detail

/*!
 * @brief Folds values from an event stream into a signal
 *
 *  Iteratively combines signal value with values from event stream.
 *  Synchronized values of signals in dep_pack are passed to func as additional arguments.
 *
 *  The signature of func should be equivalent to:
 *  * S func(const E& event, const S& accum, const Deps& ...)
 *  * S func(event_range<E> range, const S& accum, const Deps& ...)
 *  * void func(const E& event, S& accum, const Deps& ...)
 *  * void func(event_range<E> range, S& accum, const Deps& ...)
 *
 *  The fold parameters:
 *    [const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack]
 *  match the corresponding arguments of the given function
 *    [const E& event_value, const S& accumulator, const Deps& ...deps]
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
template <typename E, typename V, typename InF, typename... Deps, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto fold(
    const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack, InF&& func )
    -> signal<S>
{
    return fold_impl( events, std::forward<V>( init ), dep_pack, std::forward<InF>( func ) );
}

/*!
 * @brief Curried version of fold(const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack, InF&& func)
 */
template <typename V, typename InF, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto fold( V&& init, const signal_pack<Deps...>& dep_pack, InF&& func )
{
    return detail::closure{
        [init = std::forward<V>( init ), dep_pack = dep_pack, func = std::forward<InF>( func )](
            auto&& source ) {
            using arg_t = decltype( source );
            static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
            return fold( std::forward<arg_t>( source ), std::move( init ), dep_pack, func );
        } };
}

/*!
 * @brief Folds values from an event stream into a signal
 *
 *  Version without synchronization with additional signals
 *
 *  See fold(const events<E>& events, V&& init, const signal_pack<Deps...>& dep_pack, InF&& func)
 */
template <typename E, typename V, typename InF, typename S = std::decay_t<V>>
UREACT_WARN_UNUSED_RESULT auto fold( const events<E>& events, V&& init, InF&& func ) -> signal<S>
{
    return fold_impl( events, std::forward<V>( init ), signal_pack<>(), std::forward<InF>( func ) );
}

/*!
 * @brief Curried version of fold(const events<E>& events, V&& init, InF&& func)
 */
template <typename V, typename InF>
UREACT_WARN_UNUSED_RESULT auto fold( V&& init, InF&& func )
{
    return detail::closure{
        [init = std::forward<V>( init ), func = std::forward<InF>( func )]( auto&& source ) {
            using arg_t = decltype( source );
            static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
            return fold( std::forward<arg_t>( source ), std::move( init ), func );
        } };
}

UREACT_END_NAMESPACE

#endif // UREACT_FOLD_HPP

UREACT_BEGIN_NAMESPACE

namespace detail
{

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

} // namespace detail

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
 * @brief Curried version of collect(const events<E>& source)
 */
template <template <typename...> class ContT>
UREACT_WARN_UNUSED_RESULT auto collect()
{
    return detail::closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return collect<ContT>( std::forward<arg_t>( source ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_COLLECT_HPP

#ifndef UREACT_COUNT_HPP
#define UREACT_COUNT_HPP


UREACT_BEGIN_NAMESPACE

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
 * @brief Curried version of count(const events<E>& source)
 */
template <typename S = size_t>
UREACT_WARN_UNUSED_RESULT auto count()
{
    return detail::closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return count<S>( std::forward<arg_t>( source ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_COUNT_HPP

#ifndef UREACT_DETAIL_REACTIVE_OP_BASE_HPP
#define UREACT_DETAIL_REACTIVE_OP_BASE_HPP


UREACT_BEGIN_NAMESPACE

namespace detail
{

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
        std::apply( attach_functor<Node>{ node }, m_deps );
    }

    template <typename Node>
    void detach( Node& node ) const
    {
        std::apply( detach_functor<Node>{ node }, m_deps );
    }

    template <typename Node, typename Functor>
    void attach_rec( const Functor& functor ) const
    {
        // Same memory layout, different func
        std::apply( reinterpret_cast<const attach_functor<Node>&>( functor ), m_deps );
    }

    template <typename Node, typename Functor>
    void detach_rec( const Functor& functor ) const
    {
        std::apply( reinterpret_cast<const detach_functor<Node>&>( functor ), m_deps );
    }

protected:
    dep_holder_t m_deps;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif //UREACT_DETAIL_REACTIVE_OP_BASE_HPP

#ifndef UREACT_FLATTEN_HPP
#define UREACT_FLATTEN_HPP


UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
class signal_node;

template <typename E>
class event_stream_node;

template <typename OuterS, typename InnerS>
class signal_flatten_node final : public signal_node<InnerS>
{
public:
    signal_flatten_node( context& context,
        std::shared_ptr<signal_node<OuterS>> outer,
        std::shared_ptr<signal_node<InnerS>> inner )
        : signal_flatten_node::signal_node( context, inner->value_ref() )
        , m_outer( std::move( outer ) )
        , m_inner( std::move( inner ) )
    {
        this->attach_to( *m_outer );
        this->attach_to( *m_inner );
    }

    ~signal_flatten_node() override
    {
        this->detach_from( *m_inner );
        this->detach_from( *m_outer );
    }

    void tick( turn_type& ) override
    {
        {
            const auto& new_inner = m_outer->value_ref().get_node();
            if( !equal_to( new_inner, m_inner ) )
            {
                // Topology has been changed
                auto old_inner = m_inner;
                m_inner = new_inner;

                this->dynamic_detach_from( *old_inner );
                this->dynamic_attach_to( *new_inner );

                return;
            }
        }

        this->pulse_if_value_changed( m_inner->value_ref() );
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
        std::shared_ptr<signal_node<OuterS>> outer,
        std::shared_ptr<event_stream_node<InnerE>> inner )
        : event_flatten_node::event_stream_node( context )
        , m_outer( std::move( outer ) )
        , m_inner( std::move( inner ) )
    {
        this->attach_to( *m_outer );
        this->attach_to( *m_inner );
    }

    ~event_flatten_node() override
    {
        this->detach_from( *m_inner );
        this->detach_from( *m_outer );
    }

    void tick( turn_type& turn ) override
    {
        this->set_current_turn_force_update( turn );
        m_inner->set_current_turn( turn );

        {
            const auto& new_inner = m_outer->value_ref().get_node();
            if( !equal_to( new_inner, m_inner ) )
            {
                new_inner->set_current_turn( turn ); // events specific

                // Topology has been changed
                auto old_inner = m_inner;
                m_inner = new_inner;

                this->dynamic_detach_from( *old_inner );
                this->dynamic_attach_to( *new_inner );

                return;
            }
        }

        this->m_events.insert(
            this->m_events.end(), m_inner->events().begin(), m_inner->events().end() );

        this->pulse_if_has_events();
    }

private:
    std::shared_ptr<signal_node<OuterS>> m_outer;
    std::shared_ptr<event_stream_node<InnerE>> m_inner;
};

/*!
 * @brief Create a new event stream by flattening a signal
 */
template <typename InnerS>
UREACT_WARN_UNUSED_RESULT auto flatten_impl( const signal<InnerS>& outer )
{
    context& context = outer.get_context();

    using value_t = typename InnerS::value_t;

    // clang-format off
    using Node =
        select_t<
            condition<is_var_signal_v<InnerS>,   signal_flatten_node<InnerS, value_t>>,
            condition<is_signal_v<InnerS>,       signal_flatten_node<InnerS, value_t>>,
            condition<is_event_source_v<InnerS>, event_flatten_node<InnerS, value_t>>,
            condition<is_event_v<InnerS>,        event_flatten_node<InnerS, value_t>>,
            signature_mismatches>;
    // clang-format on

    static_assert( !std::is_same_v<Node, signature_mismatches>,
        "flatten: Passed signal does not match any of the supported signatures" );

    return InnerS{ std::make_shared<Node>( context, outer.get_node(), outer.get().get_node() ) };
}

} // namespace detail

/*!
 * @brief Create a new event stream by flattening a signal
 */
template <typename InnerS>
UREACT_WARN_UNUSED_RESULT auto flatten( const signal<InnerS>& outer )
{
    return detail::flatten_impl( outer );
}

/*!
 * @brief Curried version of flatten(const signal<signal<InnerS>>& outer)
 */
UREACT_WARN_UNUSED_RESULT inline auto flatten()
{
    return detail::closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_signal_v<std::decay_t<arg_t>>, "Signal type is required" );
        return flatten( std::forward<arg_t>( source ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_FLATTEN_HPP

#ifndef UREACT_HOLD_HPP
#define UREACT_HOLD_HPP


UREACT_BEGIN_NAMESPACE

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
 * @brief Curried version of hold()
 */
template <typename V>
UREACT_WARN_UNUSED_RESULT auto hold( V&& init )
{
    return detail::closure{ [init = std::forward<V>( init )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return hold( std::forward<arg_t>( source ), std::move( init ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_HOLD_HPP

#ifndef UREACT_LIFT_HPP
#define UREACT_LIFT_HPP


#ifndef UREACT_TEMP_SIGNAL_HPP
#define UREACT_TEMP_SIGNAL_HPP


UREACT_BEGIN_NAMESPACE

namespace detail
{

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

} // namespace detail

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

UREACT_END_NAMESPACE

#endif //UREACT_TEMP_SIGNAL_HPP

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
using signal_node_ptr_t = std::shared_ptr<signal_node<S>>;

template <typename S, typename F, typename... Deps>
class function_op : public reactive_op_base<Deps...>
{
public:
    template <typename InF, typename... Args>
    explicit function_op( InF&& func, Args&&... args )
        : function_op::reactive_op_base( dont_move(), std::forward<Args>( args )... )
        , m_func( std::forward<InF>( func ) )
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
            return std::invoke( m_func, eval( std::forward<T>( args ) )... );
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

// TODO: remove r-value reference only, leave simple references
template <typename SIn, typename F, typename... Values>
using deduce_s = std::conditional_t<std::is_same_v<SIn, void>, //
    std::decay_t<std::invoke_result_t<F, Values...>>,
    SIn>;

// Based on the transparent functor std::negate<>
struct unary_plus
{
    template <typename T>
    constexpr auto operator()( T&& t ) const          //
        noexcept( noexcept( +std::forward<T>( t ) ) ) //
        -> decltype( +std::forward<T>( t ) )
    {
        return +std::forward<T>( t );
    }

    using is_transparent = void;
};

} // namespace detail

/*!
 * @brief Create a new signal node with value v = std::invoke(func, arg_pack.get(), ...)
 * @tparam Values types of signal values
 *
 * This value is set on construction and updated when any args have changed
 */
template <typename SIn = void, typename... Values, typename InF>
UREACT_WARN_UNUSED_RESULT auto lift( const signal_pack<Values...>& arg_pack, InF&& func )
{
    using F = std::decay_t<InF>;
    using S = detail::deduce_s<SIn, F, Values...>;
    using Op = detail::function_op<S, F, detail::signal_node_ptr_t<Values>...>;

    context& context = std::get<0>( arg_pack.data ).get_context();

    auto node_builder = [&context, &func]( const signal<Values>&... args ) {
        return temp_signal<S, Op>{ context, std::forward<InF>( func ), args.get_node()... };
    };

    return std::apply( node_builder, arg_pack.data );
}

/*!
 * @brief Create a new signal node with value v = std::invoke(func, arg.get())
 *
 * This value is set on construction and updated when arg have changed
 */
template <typename SIn = void, typename Value, typename InF>
UREACT_WARN_UNUSED_RESULT auto lift( const signal<Value>& arg, InF&& func )
{
    using F = std::decay_t<InF>;
    using S = detail::deduce_s<SIn, F, Value>;
    using Op = detail::function_op<S, F, detail::signal_node_ptr_t<Value>>;
    return temp_signal<S, Op>{ arg.get_context(), std::forward<InF>( func ), arg.get_node() };
}

/*!
 * @brief Create a new signal node with value v = std::invoke(func, arg.get())
 *
 * This value is set on construction and updated when arg have changed
 */
template <typename SIn = void, typename Value, typename OpIn, typename InF>
UREACT_WARN_UNUSED_RESULT auto lift( temp_signal<Value, OpIn>&& arg, InF&& func )
{
    using F = std::decay_t<InF>;
    using S = detail::deduce_s<SIn, F, Value>;
    using Op = detail::function_op<S, F, OpIn>;
    return temp_signal<S, Op>{
        arg.get_context(), std::forward<InF>( func ), std::move( arg ).steal_op() };
}

/*!
 * @brief Curried version of lift(const signal_pack<Values...>& arg_pack, InF&& func)
 */
template <typename SIn = void, typename InF>
UREACT_WARN_UNUSED_RESULT auto lift( InF&& func )
{
    return detail::closure{ [func = std::forward<InF>( func )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert(
            is_signal_or_pack_v<std::decay_t<arg_t>>, "Signal type or signal_pack is required" );
        return lift<SIn>( std::forward<arg_t>( source ), func );
    } };
}

/*!
 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
 *
 * This value is set on construction and updated when arg have changed
 */
template <typename SIn = void,
    typename LeftSignal,
    typename InF,
    typename RightSignal,
    class = std::enable_if_t<is_signal_v<LeftSignal>>,
    class = std::enable_if_t<is_signal_v<RightSignal>>>
UREACT_WARN_UNUSED_RESULT auto lift( const LeftSignal& lhs, InF&& func, const RightSignal& rhs )
{
    return lift<SIn>( with( lhs, rhs ), std::forward<InF>( func ) );
}

/*!
 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
 *
 * This value is set on construction and updated when arg have changed
 */
template <typename SIn = void,
    typename LeftSignal,
    typename InF,
    typename RightVal,
    class = std::enable_if_t<is_signal_v<std::decay_t<LeftSignal>>>,
    class = std::enable_if_t<!is_signal_v<std::decay_t<RightVal>>>>
UREACT_WARN_UNUSED_RESULT auto lift( LeftSignal&& lhs, InF&& func, RightVal&& rhs )
{
    return lift<SIn>( std::forward<LeftSignal>( lhs ),
        std::bind(
            std::forward<InF>( func ), std::placeholders::_1, std::forward<RightVal>( rhs ) ) );
}

/*!
 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
 *
 * This value is set on construction and updated when arg have changed
 */
template <typename SIn = void,
    typename LeftVal,
    typename InF,
    typename RightSignal,
    class = std::enable_if_t<!is_signal_v<std::decay_t<LeftVal>>>,
    class = std::enable_if_t<is_signal_v<std::decay_t<RightSignal>>>,
    typename Wtf = void>
UREACT_WARN_UNUSED_RESULT auto lift( LeftVal&& lhs, InF&& func, RightSignal&& rhs )
{
    return lift<SIn>( std::forward<RightSignal>( rhs ),
        std::bind(
            std::forward<InF>( func ), std::forward<LeftVal>( lhs ), std::placeholders::_1 ) );
}

/*!
 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
 *
 * This value is set on construction and updated when arg have changed
 */
template <typename SIn = void,
    typename LeftVal,
    typename LeftOp,
    typename InF,
    typename RightVal,
    typename RightOp>
UREACT_WARN_UNUSED_RESULT auto lift(
    temp_signal<LeftVal, LeftOp>&& lhs, InF&& func, temp_signal<RightVal, RightOp>&& rhs )
{
    using F = std::decay_t<InF>;
    using S = detail::deduce_s<SIn, F, LeftVal, RightVal>;
    using Op = detail::function_op<S, F, LeftOp, RightOp>;

    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return temp_signal<S, Op>{ context,
        std::forward<InF>( func ),
        std::move( lhs ).steal_op(),
        std::move( rhs ).steal_op() };
}

/*!
 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
 *
 * This value is set on construction and updated when arg have changed
 */
template <typename SIn = void,
    typename LeftVal,
    typename LeftOp,
    typename InF,
    typename RightSignal,
    class = std::enable_if_t<is_signal_v<RightSignal>>>
UREACT_WARN_UNUSED_RESULT auto lift(
    temp_signal<LeftVal, LeftOp>&& lhs, InF&& func, const RightSignal& rhs )
{
    using RightVal = typename RightSignal::value_t;
    using F = std::decay_t<InF>;
    using S = detail::deduce_s<SIn, F, LeftVal, RightVal>;
    using Op = detail::function_op<S, F, LeftOp, detail::signal_node_ptr_t<RightVal>>;

    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return temp_signal<S, Op>{
        context, std::forward<InF>( func ), std::move( lhs ).steal_op(), rhs.get_node() };
}

/*!
 * @brief Create a new signal node with value v = std::invoke(func, lhs.get(), rhs.get())
 *
 * This value is set on construction and updated when arg have changed
 */
template <typename SIn = void,
    typename LeftSignal,
    typename InF,
    typename RightVal,
    typename RightOp,
    class = std::enable_if_t<is_signal_v<LeftSignal>>>
UREACT_WARN_UNUSED_RESULT auto lift(
    const LeftSignal& lhs, InF&& func, temp_signal<RightVal, RightOp>&& rhs )
{
    using LeftVal = typename LeftSignal::value_t;
    using F = std::decay_t<InF>;
    using S = detail::deduce_s<SIn, F, LeftVal, RightVal>;
    using Op = detail::function_op<S, F, detail::signal_node_ptr_t<LeftVal>, RightOp>;

    context& context = lhs.get_context();
    assert( context == rhs.get_context() );

    return temp_signal<S, Op>{
        context, std::forward<InF>( func ), lhs.get_node(), std::move( rhs ).steal_op() };
}

#define UREACT_DECLARE_UNARY_LIFT_OPERATOR( op, fn )                                               \
    template <typename Signal, class = std::enable_if_t<is_signal_v<std::decay_t<Signal>>>>        \
    UREACT_WARN_UNUSED_RESULT auto operator op( Signal&& arg )                                     \
    {                                                                                              \
        return lift( std::forward<Signal>( arg ), fn{} );                                          \
    }

#define UREACT_DECLARE_BINARY_LIFT_OPERATOR( op, fn )                                              \
    template <typename Lhs,                                                                        \
        typename Rhs,                                                                              \
        class = std::enable_if_t<                                                                  \
            std::disjunction_v<is_signal<std::decay_t<Lhs>>, is_signal<std::decay_t<Rhs>>>>>       \
    UREACT_WARN_UNUSED_RESULT auto operator op( Lhs&& lhs, Rhs&& rhs )                             \
    {                                                                                              \
        return lift( std::forward<Lhs>( lhs ), fn{}, std::forward<Rhs>( rhs ) );                   \
    }

// arithmetic operators

UREACT_DECLARE_BINARY_LIFT_OPERATOR( +, std::plus<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( -, std::minus<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( *, std::multiplies<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( /, std::divides<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( %, std::modulus<> )
UREACT_DECLARE_UNARY_LIFT_OPERATOR( +, detail::unary_plus )
UREACT_DECLARE_UNARY_LIFT_OPERATOR( -, std::negate<> )

// relational operators

UREACT_DECLARE_BINARY_LIFT_OPERATOR( ==, std::equal_to<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( !=, std::not_equal_to<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( <, std::less<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( <=, std::less_equal<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( >, std::greater<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( >=, std::greater_equal<> )

// logical operators

UREACT_DECLARE_BINARY_LIFT_OPERATOR( &&, std::logical_and<> )
UREACT_DECLARE_BINARY_LIFT_OPERATOR( ||, std::logical_or<> )
UREACT_DECLARE_UNARY_LIFT_OPERATOR( !, std::logical_not<> )

#undef UREACT_DECLARE_UNARY_LIFT_OPERATOR
#undef UREACT_DECLARE_BINARY_LIFT_OPERATOR

UREACT_END_NAMESPACE

#endif // UREACT_LIFT_HPP

#ifndef UREACT_MERGE_HPP
#define UREACT_MERGE_HPP


UREACT_BEGIN_NAMESPACE

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

    UREACT_MAKE_MOVABLE( event_merge_op );

    template <typename Collector>
    void collect( const turn_type& turn, const Collector& collector ) const
    {
        std::apply( collect_functor<Collector>( turn, collector ), this->m_deps );
    }

    template <typename Collector, typename Functor>
    void collect_rec( const Functor& functor ) const
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
        void collect( const T& op ) const
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

template <typename E>
class event_stream_node;

template <typename E, typename Op>
class event_op_node;

template <typename E>
using event_stream_node_ptr_t = std::shared_ptr<event_stream_node<E>>;

} // namespace detail

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

UREACT_END_NAMESPACE

#endif // UREACT_MERGE_HPP

#ifndef UREACT_OBSERVE_HPP
#define UREACT_OBSERVE_HPP

#include <functional>


#ifndef UREACT_OBSERVER_HPP
#define UREACT_OBSERVER_HPP


UREACT_BEGIN_NAMESPACE

/*!
 * @brief Observer functions can return values of this type to control further processing.
 */
enum class observer_action
{
    next,           ///< Continue observing
    stop_and_detach ///< Stop observing
};

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
    UREACT_MAKE_NONCOPYABLE( observer );
    UREACT_MAKE_MOVABLE( observer );

    /*!
     * @brief Default constructor
     */
    observer() = default;

    /*!
     * @brief Construct from the give node and a subject.
     * 
     * Not intended to be used directly. Use `observe()` instead.
     */
    observer( Node* node, subject_ptr_t subject )
        : m_node( node )
        , m_subject( std::move( subject ) )
    {}

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
    /// Owned by subject
    Node* m_node = nullptr;

    /// While the observer handle exists, the subject is not destroyed
    subject_ptr_t m_subject = nullptr;
};

UREACT_END_NAMESPACE

#endif //UREACT_OBSERVER_HPP

UREACT_BEGIN_NAMESPACE

template <typename E>
class event_range;

namespace detail
{

template <typename E>
class event_stream_node;

/// Special wrapper to add specific return type to the void function
template <typename F, typename Ret, Ret return_value>
class add_default_return_value_wrapper
{
public:
    template <typename InF, class = disable_if_same_t<InF, add_default_return_value_wrapper>>
    explicit add_default_return_value_wrapper( InF&& func )
        : m_func( std::forward<InF>( func ) )
    {}

    template <typename... Args>
    UREACT_WARN_UNUSED_RESULT Ret operator()( Args&&... args )
    {
        std::invoke( m_func, std::forward<Args>( args )... );
        return return_value;
    }

private:
    F m_func;
};

template <class F>
using add_observer_action_next_ret
    = add_default_return_value_wrapper<F, observer_action, observer_action::next>;

template <typename E, typename F, typename... Args>
class add_observer_range_wrapper
{
public:
    template <typename InF, class = disable_if_same_t<InF, add_observer_range_wrapper>>
    explicit add_observer_range_wrapper( InF&& func )
        : m_func( std::forward<InF>( func ) )
    {}

    // NOTE: args can't be universal reference since its type is specified in class
    // NOTE: can't be const because m_func can be mutable
    observer_action operator()( event_range<E> range, const Args&... args )
    {
        for( const auto& e : range )
        {
            if( std::invoke( m_func, e, args... ) == observer_action::stop_and_detach )
            {
                return observer_action::stop_and_detach;
            }
        }

        return observer_action::next;
    }

private:
    F m_func;
};

template <typename S>
class signal_node;

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
        this->attach_to( *subject );
    }

    void tick( turn_type& ) override
    {
        bool should_detach = false;

        if( auto p = m_subject.lock() )
        {
            if( std::invoke( m_func, p->value_ref() ) == observer_action::stop_and_detach )
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
            detach_from( *p );
            m_subject.reset();
        }
    }

    std::weak_ptr<signal_node<S>> m_subject;
    F m_func;
};

template <typename E, typename F, typename... Deps>
class events_observer_node final : public observer_node
{
public:
    template <typename InF>
    events_observer_node( context& context,
        const std::shared_ptr<event_stream_node<E>>& subject,
        InF&& func,
        const std::shared_ptr<signal_node<Deps>>&... deps )
        : events_observer_node::observer_node( context )
        , m_subject( subject )
        , m_func( std::forward<InF>( func ) )
        , m_deps( deps... )
    {
        this->attach_to( *subject );
        ( this->attach_to( *deps ), ... );
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
                          [this, &p]( const std::shared_ptr<signal_node<Deps>>&... args ) {
                              return std::invoke(
                                  m_func, event_range<E>( p->events() ), args->value_ref()... );
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
    using DepHolder = std::tuple<std::shared_ptr<signal_node<Deps>>...>;

    std::weak_ptr<event_stream_node<E>> m_subject;

    F m_func;
    DepHolder m_deps;

    void detach_observer() override
    {
        if( auto p = m_subject.lock() )
        {
            detach_from( *p );

            std::apply( detach_functor<events_observer_node>( *this ), m_deps );

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

template <typename InF, typename E, typename... Deps>
auto observe_events_impl(
    const events<E>& subject, const signal_pack<Deps...>& dep_pack, InF&& func ) -> observer
{
    using F = std::decay_t<InF>;

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
            context, subject.get_node(), std::forward<InF>( func ), deps.get_node()... );
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

/*!
 * @brief Curried version of observe(T&& subject, F&& func)
 */
template <typename F>
UREACT_WARN_UNUSED_RESULT auto observe( F&& func ) // TODO: check in tests
{
    // TODO: propagate [[nodiscard]] to closure operator() and operator|
    //       they should not be nodiscard for l-value arguments, but only for r-values like observe() does
    //       but maybe all observe() concept should be reconsidered before to not do feature that is possibly not needed
    return detail::closure{ [func = std::forward<F>( func )]( auto&& subject ) {
        using arg_t = decltype( subject );
        static_assert(
            is_observable_v<std::decay_t<arg_t>>, "Observable type is required (signal or event)" );
        return observe( std::forward<arg_t>( subject ), func );
    } };
}

/*!
 * @brief Curried version of observe(T&& subject, const signal_pack<Deps...>& dep_pack, F&& func)
 */
template <typename F, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto observe(
    const signal_pack<Deps...>& dep_pack, F&& func ) // TODO: check in tests
{
    return detail::closure{
        [dep_pack = dep_pack, func = std::forward<F>( func )]( auto&& subject ) {
            using arg_t = decltype( subject );
            static_assert( is_observable_v<std::decay_t<arg_t>>,
                "Observable type is required (signal or event)" );
            return observe( std::forward<arg_t>( subject ), dep_pack, func );
        } };
}

UREACT_END_NAMESPACE

#endif // UREACT_OBSERVE_HPP

#ifndef UREACT_PULSE_HPP
#define UREACT_PULSE_HPP


UREACT_BEGIN_NAMESPACE

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
                out << target_value;
        } );
}

/*!
 * @brief Curried version of pulse()
 */
template <typename S>
UREACT_WARN_UNUSED_RESULT auto pulse( const signal<S>& target )
{
    return detail::closure{ [target = target]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return pulse( std::forward<arg_t>( source ), target );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_PULSE_HPP

#ifndef UREACT_REACTIVE_REF_HPP
#define UREACT_REACTIVE_REF_HPP


UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename T>
struct decay_input
{
    using type = T;
};

template <typename S>
struct decay_input<var_signal<S>>
{
    using type = signal<S>;
};

template <typename Owner, typename S>
struct decay_input<member_signal<Owner, S>>
{
    using type = signal<S>;
};

template <typename Owner, typename S>
struct decay_input<member_var_signal<Owner, S>>
{
    using type = signal<S>;
};

// TODO: replace it with something more appropriate or at least name it
template <typename T>
using decay_input_t = typename decay_input<T>::type;

} // namespace detail

/*!
 * @brief Utility to flatten public signal attribute of class pointed be reference
 *
 *  For example we have a class Foo with a public signal bar: struct Foo{ signal<int> bar; };
 *  Also, we have signal that points to this class by pointer: signal<Foo*> bar
 *  This utility receives a signal pointer bar and attribute pointer &Foo::bar and flattens it to signal<int> foobar
 */
template <typename Signal,
    typename InF,
    class = std::enable_if_t<is_signal_v<std::decay_t<Signal>>>>
UREACT_WARN_UNUSED_RESULT auto reactive_ref( Signal&& outer, InF&& func )
{
    using S = typename std::decay_t<Signal>::value_t;
    using F = std::decay_t<InF>;
    using R = std::invoke_result_t<F, S>;
    using DecayedR = detail::decay_input_t<std::decay_t<R>>;
    return flatten( lift<DecayedR>( std::forward<Signal>( outer ), std::forward<InF>( func ) ) );
}

UREACT_END_NAMESPACE

#endif // UREACT_REACTIVE_REF_HPP

#ifndef UREACT_SCOPED_OBSERVER_HPP
#define UREACT_SCOPED_OBSERVER_HPP


UREACT_BEGIN_NAMESPACE

/*!
 * @brief Takes ownership of an observer and automatically detaches it on scope exit
 */
class scoped_observer final
{
public:
    UREACT_MAKE_NONCOPYABLE( scoped_observer );
    UREACT_MAKE_MOVABLE( scoped_observer );

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

UREACT_END_NAMESPACE

#endif //UREACT_SCOPED_OBSERVER_HPP

#ifndef UREACT_SNAPSHOT_HPP
#define UREACT_SNAPSHOT_HPP


UREACT_BEGIN_NAMESPACE

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
 * @brief Curried version of snapshot()
 */
template <typename S>
UREACT_WARN_UNUSED_RESULT auto snapshot( const signal<S>& target )
{
    return detail::closure{ [target = target]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return snapshot( std::forward<arg_t>( source ), target );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_SNAPSHOT_HPP

#ifndef UREACT_TAKE_DROP_HPP
#define UREACT_TAKE_DROP_HPP


UREACT_BEGIN_NAMESPACE

namespace detail
{

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

} // namespace detail

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
 * @brief Curried version of take(const events<E>& source, const size_t count)
 */
template <typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT auto take( const N count )
{
    assert( count >= 0 );
    return detail::closure{ [count]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return take( std::forward<arg_t>( source ), count );
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
 * @brief Curried version of drop(const events<E>& source, const N count)
 */
template <typename N, class = std::enable_if_t<std::is_integral_v<N>>>
UREACT_WARN_UNUSED_RESULT auto drop( const N count )
{
    assert( count >= 0 );
    return detail::closure{ [count]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return drop( std::forward<arg_t>( source ), count );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_TAKE_DROP_HPP

#ifndef UREACT_TAKE_DROP_WHILE_HPP
#define UREACT_TAKE_DROP_WHILE_HPP


UREACT_BEGIN_NAMESPACE

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
    return filter( source,
        dep_pack,
        [passed = true, pred = std::forward<Pred>( pred )] //
        ( const auto& e, const auto... deps ) mutable {
            passed = passed && std::invoke( pred, e, deps... );
            return passed;
        } );
}

/*!
 * @brief Curried version of take_while(const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred)
 */
template <typename... Deps, typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto take_while(
    const signal_pack<Deps...>& dep_pack, Pred&& pred )
{
    return detail::closure{ [dep_pack = dep_pack, pred = std::forward<Pred>( pred )] //
        ( auto&& source ) {
            using arg_t = decltype( source );
            static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
            return take_while( std::forward<arg_t>( source ), dep_pack, pred );
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
 * @brief Curried version of take_while(const events<E>& source, Pred&& pred)
 */
template <typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto take_while( Pred&& pred )
{
    return detail::closure{ [pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return take_while( std::forward<arg_t>( source ), pred );
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
    return filter( source,
        dep_pack,
        [passed = false, pred = std::forward<Pred>( pred )] //
        ( const auto& e, const auto... deps ) mutable {
            passed = passed || !std::invoke( pred, e, deps... );
            return passed;
        } );
}

/*!
 * @brief Curried version of drop_while(const events<E>& source, const signal_pack<Deps...>& dep_pack, Pred&& pred)
 */
template <typename... Deps, typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto drop_while(
    const signal_pack<Deps...>& dep_pack, Pred&& pred )
{
    return detail::closure{
        [dep_pack = dep_pack, pred = std::forward<Pred>( pred )]( auto&& source ) {
            using arg_t = decltype( source );
            static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
            return drop_while( std::forward<arg_t>( source ), dep_pack, pred );
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
 * @brief Curried version of drop_while(const events<E>& source, Pred&& pred)
 */
template <typename Pred>
UREACT_WARN_UNUSED_RESULT inline auto drop_while( Pred&& pred )
{
    return detail::closure{ [pred = std::forward<Pred>( pred )]( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return drop_while( std::forward<arg_t>( source ), pred );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_TAKE_DROP_WHILE_HPP

#ifndef UREACT_TAP_HPP
#define UREACT_TAP_HPP


UREACT_BEGIN_NAMESPACE

/*!
 * @brief Create observer for signal and return observed signal
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
 */
template <typename F,
    typename Signal, //
    class = std::enable_if_t<is_signal_v<std::decay_t<Signal>>>>
UREACT_WARN_UNUSED_RESULT auto tap( Signal&& subject, F&& func )
{
    std::ignore = observe_signal_impl( subject, std::forward<F>( func ) );
    return std::forward<Signal>( subject );
}

/*!
 * @brief Create observer for event stream and return observed event stream
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
 *  @note The event_range<E> option allows to explicitly batch process single turn events
 *  @note Changes of signals in dep_pack do not trigger an update - only received events do
 */
template <typename F,
    typename Events,
    typename... Deps,
    class = std::enable_if_t<is_event_v<std::decay_t<Events>>>>
UREACT_WARN_UNUSED_RESULT auto tap(
    Events&& subject, const signal_pack<Deps...>& dep_pack, F&& func )
{
    std::ignore = observe_events_impl( subject, dep_pack, std::forward<F>( func ) );
    return std::forward<Events>( subject );
}

/*!
 * @brief Create observer for event stream and return observed event stream
 *
 *  Version without synchronization with additional signals
 *
 *  See tap(Events&& subject, const signal_pack<Deps...>& dep_pack, F&& func)
 */
template <typename F,
    typename Events,
    int wtf = 0, // hack to resolve ambiguity with signal version of tap
    class = std::enable_if_t<is_event_v<std::decay_t<Events>>>>
UREACT_WARN_UNUSED_RESULT auto tap( Events&& subject, F&& func )
{
    std::ignore = observe_events_impl( subject, signal_pack<>{}, std::forward<F>( func ) );
    return std::forward<Events>( subject );
}

/*!
 * @brief Curried version of tap(T&& subject, F&& func)
 */
template <typename F>
UREACT_WARN_UNUSED_RESULT auto tap( F&& func )
{
    return detail::closure{ [func = std::forward<F>( func )]( auto&& subject ) {
        using arg_t = decltype( subject );
        static_assert(
            is_observable_v<std::decay_t<arg_t>>, "Observable type is required (signal or event)" );
        return tap( std::forward<arg_t>( subject ), func );
    } };
}

/*!
 * @brief Curried version of tap(T&& subject, const signal_pack<Deps...>& dep_pack, F&& func)
 */
template <typename F, typename... Deps>
UREACT_WARN_UNUSED_RESULT auto tap( const signal_pack<Deps...>& dep_pack, F&& func )
{
    return detail::closure{
        [dep_pack = dep_pack, func = std::forward<F>( func )]( auto&& subject ) {
            using arg_t = decltype( subject );
            static_assert( is_observable_v<std::decay_t<arg_t>>,
                "Observable type is required (signal or event)" );
            return tap( std::forward<arg_t>( subject ), dep_pack, func );
        } };
}

UREACT_END_NAMESPACE

#endif // UREACT_TAP_HPP

#ifndef UREACT_TRANSACTION_HPP
#define UREACT_TRANSACTION_HPP

#include <functional>


UREACT_BEGIN_NAMESPACE

/*!
 * @brief Guard class to perform several changes atomically
 *
 */
class UREACT_WARN_UNUSED_RESULT transaction
{
public:
    explicit transaction( context& ctx )
        : self( _get_internals( ctx ).get_graph() )
    {
        ++self.m_transaction_level;
    }

    ~transaction()
    {
        --self.m_transaction_level;

        if( self.m_transaction_level == 0 )
        {
            self.finalize_transaction();
        }
    }

private:
    UREACT_MAKE_NONCOPYABLE( transaction );
    UREACT_MAKE_NONMOVABLE( transaction );

    detail::react_graph& self;
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
    transaction _{ ctx };

    if constexpr( std::is_same_v<std::invoke_result_t<F&&, Args&&...>, void> )
    {
        std::invoke( std::forward<F>( func ), std::forward<Args>( args )... );
    }
    else
    {
        return std::invoke( std::forward<F>( func ), std::forward<Args>( args )... );
    }
}

UREACT_END_NAMESPACE

#endif // UREACT_TRANSACTION_HPP

#ifndef UREACT_UNIQUE_HPP
#define UREACT_UNIQUE_HPP


UREACT_BEGIN_NAMESPACE

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
 * @brief Curried version of unique(const events<E>& source)
 */
UREACT_WARN_UNUSED_RESULT inline auto unique()
{
    return detail::closure{ []( auto&& source ) {
        using arg_t = decltype( source );
        static_assert( is_event_v<std::decay_t<arg_t>>, "Event type is required" );
        return unique( std::forward<arg_t>( source ) );
    } };
}

UREACT_END_NAMESPACE

#endif // UREACT_UNIQUE_HPP

#ifndef UREACT_VERSION_HPP
#define UREACT_VERSION_HPP

#define UREACT_VERSION_MAJOR 0
#define UREACT_VERSION_MINOR 8
#define UREACT_VERSION_PATCH 1
#define UREACT_VERSION_STR "0.8.1"

#define UREACT_VERSION                                                                             \
    ( UREACT_VERSION_MAJOR * 10000 + UREACT_VERSION_MINOR * 100 + UREACT_VERSION_PATCH )

#endif //UREACT_VERSION_HPP

#ifndef UREACT_ZIP_HPP
#define UREACT_ZIP_HPP

#include <deque>


UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename E>
class event_stream_node;

template <typename... Values>
class event_zip_node final : public event_stream_node<std::tuple<Values...>>
{
public:
    explicit event_zip_node(
        context& context, const std::shared_ptr<event_stream_node<Values>>&... sources )
        : event_zip_node::event_stream_node( context )
        , m_slots( sources... )
    {
        ( this->attach_to( *sources ), ... );
    }

    ~event_zip_node() override
    {
        std::apply(
            [this]( slot<Values>&... slots ) { ( this->detach_from( *slots.source ), ... ); },
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

        this->pulse_if_has_events();
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

} // namespace detail

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

UREACT_END_NAMESPACE

#endif // UREACT_ZIP_HPP
#endif // UREACT_UREACT_AMALGAMATED_HPP
