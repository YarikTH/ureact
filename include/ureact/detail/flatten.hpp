#pragma once

#include "ureact/detail/graph/flatten_node.hpp"

namespace ureact
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// flatten
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename inner_value_t>
auto flatten( const signal<signal<inner_value_t>>& outer ) -> signal<inner_value_t>
{
    context* context = outer.get_context();
    return signal<inner_value_t>(
        context,
        std::make_shared<::ureact::detail::flatten_node<signal<inner_value_t>, inner_value_t>>(
            context, get_node_ptr(outer), get_node_ptr(outer.value())));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// flatten macros
///////////////////////////////////////////////////////////////////////////////////////////////////
// Note: Using static_cast rather than -> return type, because when using lambda for inline
// class initialization, decltype did not recognize the parameter r
// Note2: MSVC doesn't like typename in the lambda
#if defined( _MSC_VER ) && _MSC_VER && !__INTEL_COMPILER
#    define REACT_MSVC_NO_TYPENAME
#else
#    define REACT_MSVC_NO_TYPENAME typename
#endif

#define REACTIVE_REF(obj, name)                                                             \
    flatten(                                                                                \
        make_signal(                                                                        \
            obj,                                                                            \
            [] (const REACT_MSVC_NO_TYPENAME                                                \
                ::ureact::detail::identity<decltype(obj)>::type::value_t& r)                \
            {                                                                               \
                using T = decltype(r.name);                                                 \
                using S = REACT_MSVC_NO_TYPENAME ::ureact::decay_input<T>::type;            \
                return static_cast<S>(r.name);                                              \
            }))

#define REACTIVE_PTR(obj, name)                                                             \
    flatten(                                                                                \
        make_signal(                                                                        \
            obj,                                                                            \
            [] (REACT_MSVC_NO_TYPENAME                                                      \
                ::ureact::detail::identity<decltype(obj)>::type::value_t r)                 \
            {                                                                               \
                assert(r != nullptr);                                                       \
                using T = decltype(r->name);                                                \
                using S = REACT_MSVC_NO_TYPENAME ::ureact::decay_input<T>::type;            \
                return static_cast<S>(r->name);                                             \
            }))

}
