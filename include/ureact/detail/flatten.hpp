#pragma once

#include "ureact/detail/graph/flatten_node.hpp"

namespace ureact
{

// Forward declaration to break cyclic dependency
template <typename S>
class signal;
template <typename S>
class var_signal;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// flatten
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename inner_value_t>
auto flatten( const signal<signal<inner_value_t>>& outer ) -> signal<inner_value_t>
{
    context& context = outer.get_context();
    return signal<inner_value_t>(
        std::make_shared<::ureact::detail::flatten_node<signal<inner_value_t>, inner_value_t>>(
            context, get_node_ptr( outer ), get_node_ptr( outer.value() ) ) );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// decay_input
///////////////////////////////////////////////////////////////////////////////////////////////////
/// @todo understand its meaning and document it
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

///////////////////////////////////////////////////////////////////////////////////////////////////
/// flatten macros
///////////////////////////////////////////////////////////////////////////////////////////////////

#define REACTIVE_REF( obj, name )                                                                  \
    flatten( make_signal(                                                                          \
        obj, []( const typename ::ureact::detail::identity<decltype( obj )>::type::value_t& r ) {  \
            using T = decltype( r.name );                                                          \
            using S = typename ::ureact::decay_input<T>::type;                                     \
            return static_cast<S>( r.name );                                                       \
        } ) )

#define REACTIVE_PTR( obj, name )                                                                  \
    flatten( make_signal(                                                                          \
        obj, []( typename ::ureact::detail::identity<decltype( obj )>::type::value_t r ) {         \
            assert( r != nullptr );                                                                \
            using T = decltype( r->name );                                                         \
            using S = typename ::ureact::decay_input<T>::type;                                     \
            return static_cast<S>( r->name );                                                      \
        } ) )

} // namespace ureact
