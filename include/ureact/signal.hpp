#pragma once

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ureact/context.hpp"
#include "ureact/observer.hpp"
#include "ureact/type_traits.hpp"
#include "ureact/detail/signal_base.hpp"
#include "ureact/detail/graph/function_op.hpp"
#include "ureact/detail/graph/signal_op_node.hpp"
#include "ureact/detail/graph/flatten_node.hpp"

namespace ureact {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class signal;

template <typename S>
class var_signal;

template <typename S, typename op_t>
class temp_signal;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_pack - Wraps several nodes in a tuple. Create with comma operator.
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename ... values_t
>
class signal_pack
{
public:
    signal_pack(const signal<values_t>&  ... deps) :
        data( std::tie(deps ...) )
    {}

    template <typename ... cur_values_t, typename append_value_t>
    signal_pack(const signal_pack<cur_values_t ...>& cur_args, const signal<append_value_t>& new_arg) :
        data( std::tuple_cat(cur_args.data, std::tie(new_arg)) )
    {}

    std::tuple<const signal<values_t>& ...> data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// with - Utility function to create a signal_pack
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename ... values_t
>
auto with(const signal<values_t>&  ... deps)
    -> signal_pack<values_t ...>
{
    return signal_pack<values_t...>(deps ...);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// make_var
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename V,
    typename S = typename std::decay<V>::type,
    class = typename std::enable_if<
        ! is_signal<S>::value>::type
>
auto make_var(context* context, V&& value)
    -> var_signal<S>
{
    return var_signal<S>(
        context,
        std::make_shared<::ureact::detail::var_node<S>>(
            context,
            std::forward<V>(value)));
}

template
<
    typename S
>
auto make_var(context* context, std::reference_wrapper<S> value)
    -> var_signal<S&>
{
    return var_signal<S&>(
        context,
        std::make_shared<::ureact::detail::var_node<std::reference_wrapper<S>>>(context, value));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// make_var (higher order reactives)
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename V,
    typename S = typename std::decay<V>::type,
    typename inner_t = typename S::value_t,
    class = typename std::enable_if<
        is_signal<S>::value>::type
>
auto make_var(context* context, V&& value)
    -> var_signal<signal<inner_t>>
{
    return var_signal<signal<inner_t>>(
        context,
        std::make_shared<::ureact::detail::var_node<signal<inner_t>>>(
            context, std::forward<V>(value)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// make_signal
///////////////////////////////////////////////////////////////////////////////////////////////////
// Single arg
template
<
    typename value_t,
    typename in_f,
    typename F = typename std::decay<in_f>::type,
    typename S = typename std::result_of<F(value_t)>::type,
    typename op_t = ::ureact::detail::function_op<S,F, ::ureact::detail::signal_node_ptr_t<value_t>>
>
auto make_signal(const signal<value_t>& arg, in_f&& func)
    -> temp_signal<S,op_t>
{
    context* context = arg.get_context();
    return temp_signal<S,op_t>(
        context,
        std::make_shared<::ureact::detail::signal_op_node<S,op_t>>(
            context, std::forward<in_f>(func), get_node_ptr(arg)));
}

// Multiple args
template
<
    typename ... values_t,
    typename in_f,
    typename F = typename std::decay<in_f>::type,
    typename S = typename std::result_of<F(values_t...)>::type,
    typename op_t = ::ureact::detail::function_op<S,F, ::ureact::detail::signal_node_ptr_t<values_t> ...>
>
auto make_signal(const signal_pack<values_t...>& arg_pack, in_f&& func)
    -> temp_signal<S,op_t>
{
    using ::ureact::detail::signal_op_node;

    struct node_builder
    {
        explicit node_builder(context* context, in_f&& func) :
            m_context( context ),
            m_my_func( std::forward<in_f>( func ) )
        {}

        auto operator()(const signal<values_t>& ... args)
            -> temp_signal<S,op_t>
        {
            return temp_signal<S,op_t>(
                m_context,
                std::make_shared<signal_op_node<S,op_t>>(
                    m_context,
                    std::forward<in_f>( m_my_func ), get_node_ptr( args ) ...));
        }

        context* m_context;
        in_f     m_my_func;
    };

    return apply(
        node_builder( std::get<0>(arg_pack.data).get_context(), std::forward<in_f>(func) ),
        arg_pack.data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Comma operator overload to create signal pack from 2 signals.
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename left_val_t,
    typename right_val_t
>
auto operator,(const signal<left_val_t>& a, const signal<right_val_t>& b)
    -> signal_pack<left_val_t, right_val_t>
{
    return signal_pack<left_val_t, right_val_t>(a, b);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Comma operator overload to append node to existing signal pack.
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename ... cur_values_t,
    typename append_value_t
>
auto operator,(const signal_pack<cur_values_t ...>& cur, const signal<append_value_t>& append)
    -> signal_pack<cur_values_t ... , append_value_t>
{
    return signal_pack<cur_values_t ... , append_value_t>(cur, append);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// operator->* overload to connect signals to a function and return the resulting signal.
///////////////////////////////////////////////////////////////////////////////////////////////////
// Single arg
template
<
    typename F,
    template <typename V_> class signal_t,
    typename value_t,
    class = typename std::enable_if<
        is_signal<signal_t<value_t>>::value>::type
>
auto operator->*(const signal_t<value_t>& arg, F&& func)
    -> signal<typename std::result_of<F(value_t)>::type>
{
    return ::ureact::make_signal(arg, std::forward<F>(func));
}

// Multiple args
template
<
    typename F,
    typename ... values_t
>
auto operator->*(const signal_pack<values_t ...>& arg_pack, F&& func)
    -> signal<typename std::result_of<F(values_t ...)>::type>
{
    return ::ureact::make_signal(arg_pack, std::forward<F>(func));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// flatten
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename inner_value_t
>
auto flatten(const signal<signal<inner_value_t>>& outer)
    -> signal<inner_value_t>
{
    context* context = outer.get_context();
    return signal<inner_value_t>(
        context,
        std::make_shared<::ureact::detail::flatten_node<signal<inner_value_t>, inner_value_t>>(
            context, get_node_ptr(outer), get_node_ptr(outer.value())));
}

}

#include "ureact/unary_operators.hpp"
#include "ureact/binary_operators.hpp"
#include "ureact/signal_.hpp"
#include "ureact/var_signal.hpp"
#include "ureact/temp_signal.hpp"

namespace ureact { namespace detail {

template <typename L, typename R>
bool equals(const signal<L>& lhs, const signal<R>& rhs)
{
    return lhs.equals(rhs);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// flatten macros
///////////////////////////////////////////////////////////////////////////////////////////////////
// Note: Using static_cast rather than -> return type, because when using lambda for inline
// class initialization, decltype did not recognize the parameter r
// Note2: MSVC doesn't like typename in the lambda
#if defined(_MSC_VER) && _MSC_VER && !__INTEL_COMPILER
    #define REACT_MSVC_NO_TYPENAME
#else
    #define REACT_MSVC_NO_TYPENAME typename
#endif

#define REACTIVE_REF(obj, name)                                                             \
    flatten(                                                                                \
        make_signal(                                                                         \
            obj,                                                                            \
            [] (const REACT_MSVC_NO_TYPENAME                                                \
                ::ureact::detail::identity<decltype(obj)>::type::value_t& r)                       \
            {                                                                               \
                using T = decltype(r.name);                                                 \
                using S = REACT_MSVC_NO_TYPENAME ::ureact::decay_input<T>::type;              \
                return static_cast<S>(r.name);                                              \
            }))

#define REACTIVE_PTR(obj, name)                                                             \
    flatten(                                                                                \
        make_signal(                                                                         \
            obj,                                                                            \
            [] (REACT_MSVC_NO_TYPENAME                                                      \
                ::ureact::detail::identity<decltype(obj)>::type::value_t r)                        \
            {                                                                               \
                assert(r != nullptr);                                                       \
                using T = decltype(r->name);                                                \
                using S = REACT_MSVC_NO_TYPENAME ::ureact::decay_input<T>::type;              \
                return static_cast<S>(r->name);                                             \
            }))

}}
