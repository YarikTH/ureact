#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Binary operators
///////////////////////////////////////////////////////////////////////////////////////////////////
#define REACT_DECLARE_OP(op,name)                                                   \
                                                                                    \
namespace ureact { namespace detail { namespace op_functors {                        \
template <typename L, typename R>                                                   \
struct op_functor_ ## name                                                            \
{                                                                                   \
    auto operator()(const L& lhs, const R& rhs) const                               \
        -> decltype(std::declval<L>() op std::declval<R>())                         \
    {                                                                               \
        return lhs op rhs;                                                          \
    }                                                                               \
};                                                                                  \
                                                                                    \
template <typename L, typename R>                                                   \
struct op_r_functor_ ## name                                                           \
{                                                                                   \
    op_r_functor_ ## name(op_r_functor_ ## name&& other) noexcept :                       \
        m_left_val( std::move(other.m_left_val) )                                         \
    {}                                                                              \
                                                                                    \
    op_r_functor_ ## name& operator=(op_r_functor_ ## name&& other) noexcept = delete;    \
                                                                                    \
    template<                                                                       \
        typename T,                                                                 \
        class = typename std::enable_if<!is_same_decay<T,op_r_functor_ ## name>::value>::type \
    >                                                                               \
    op_r_functor_ ## name(T&& val) :                                                   \
        m_left_val( std::forward<T>(val) )                                             \
    {}                                                                              \
                                                                                    \
    op_r_functor_ ## name(const op_r_functor_ ## name& other) = delete;                   \
                                                                                    \
    op_r_functor_ ## name& operator=(const op_r_functor_ ## name& other) = delete;        \
                                                                                    \
    ~op_r_functor_ ## name() = default;                                                \
                                                                                    \
    auto operator()(const R& rhs) const                                             \
        -> decltype(std::declval<L>() op std::declval<R>())                         \
    {                                                                               \
        return m_left_val op rhs;                                                      \
    }                                                                               \
                                                                                    \
    L m_left_val;                                                                      \
};                                                                                  \
                                                                                    \
template <typename L, typename R>                                                   \
struct op_l_functor_ ## name                                                           \
{                                                                                   \
    op_l_functor_ ## name(op_l_functor_ ## name&& other) noexcept :                       \
        m_right_val( std::move(other.m_right_val) )                                       \
    {}                                                                              \
                                                                                    \
    op_l_functor_ ## name& operator=(op_l_functor_ ## name&& other) noexcept = delete;    \
                                                                                    \
    template<                                                                       \
        typename T,                                                                 \
        class = typename std::enable_if<!is_same_decay<T,op_l_functor_ ## name>::value>::type \
    >                                                                               \
    op_l_functor_ ## name(T&& val) :                                                   \
        m_right_val( std::forward<T>(val) )                                            \
    {}                                                                              \
                                                                                    \
    op_l_functor_ ## name(const op_l_functor_ ## name& other) = delete;                   \
                                                                                    \
    op_l_functor_ ## name& operator=(const op_l_functor_ ## name& other) = delete;        \
                                                                                    \
    ~op_l_functor_ ## name() = default;                                                \
                                                                                    \
    auto operator()(const L& lhs) const                                             \
        -> decltype(std::declval<L>() op std::declval<R>())                         \
    {                                                                               \
        return lhs op m_right_val;                                                     \
    }                                                                               \
                                                                                    \
    R m_right_val;                                                                     \
};                                                                                  \
}}}                                                                                  \
                                                                                    \
namespace ureact {                                                                   \
template                                                                            \
<                                                                                   \
    typename left_signal_t,                                                           \
    typename right_signal_t,                                                          \
    typename left_val_t = typename left_signal_t::value_t,                               \
    typename right_val_t = typename right_signal_t::value_t,                             \
    class = typename std::enable_if<                                                \
        is_signal<left_signal_t>::value>::type,                                        \
    class = typename std::enable_if<                                                \
        is_signal<right_signal_t>::value>::type,                                       \
    typename F = ::ureact::detail::op_functors::op_functor_ ## name<left_val_t,right_val_t>,              \
    typename S = typename std::result_of<F(left_val_t,right_val_t)>::type,              \
    typename op_t = ::ureact::detail::function_op<S,F,                                   \
        ::ureact::detail::signal_node_ptr_t<left_val_t>,                                    \
        ::ureact::detail::signal_node_ptr_t<right_val_t>>                                   \
>                                                                                   \
auto operator op(const left_signal_t& lhs, const right_signal_t& rhs)                   \
    -> temp_signal<S,op_t>                                                            \
{                                                                                   \
    context* context = lhs.get_context();                                            \
    assert(context == rhs.get_context());                                            \
    return temp_signal<S,op_t>(                                                       \
        context,                                                                    \
        std::make_shared<::ureact::detail::signal_op_node<S,op_t>>(                       \
            context, F(), get_node_ptr(lhs), get_node_ptr(rhs)));                       \
}                                                                                   \
                                                                                    \
template                                                                            \
<                                                                                   \
    typename left_signal_t,                                                           \
    typename right_val_in_t,                                                           \
    typename left_val_t = typename left_signal_t::value_t,                               \
    typename right_val_t = typename std::decay<right_val_in_t>::type,                    \
    class = typename std::enable_if<                                                \
        is_signal<left_signal_t>::value>::type,                                        \
    class = typename std::enable_if<                                                \
        ! is_signal<right_val_t>::value>::type,                                        \
    typename F = ::ureact::detail::op_functors::op_l_functor_ ## name<left_val_t,right_val_t>,             \
    typename S = typename std::result_of<F(left_val_t)>::type,                        \
    typename op_t = ::ureact::detail::function_op<S,F,                                   \
        ::ureact::detail::signal_node_ptr_t<left_val_t>>                                    \
>                                                                                   \
auto operator op(const left_signal_t& lhs, right_val_in_t&& rhs)                         \
    -> temp_signal<S,op_t>                                                            \
{                                                                                   \
    context* context = lhs.get_context();                                            \
    return temp_signal<S,op_t>(                                                       \
        context,                                                                    \
        std::make_shared<::ureact::detail::signal_op_node<S,op_t>>(                       \
            context, F( std::forward<right_val_in_t>(rhs) ), get_node_ptr(lhs)));        \
}                                                                                   \
                                                                                    \
template                                                                            \
<                                                                                   \
    typename left_val_in_t,                                                            \
    typename right_signal_t,                                                          \
    typename left_val_t = typename std::decay<left_val_in_t>::type,                      \
    typename right_val_t = typename right_signal_t::value_t,                             \
    class = typename std::enable_if<                                                \
        ! is_signal<left_val_t>::value>::type,                                         \
    class = typename std::enable_if<                                                \
        is_signal<right_signal_t>::value>::type,                                       \
    typename F = ::ureact::detail::op_functors::op_r_functor_ ## name<left_val_t,right_val_t>,             \
    typename S = typename std::result_of<F(right_val_t)>::type,                       \
    typename op_t = ::ureact::detail::function_op<S,F,                                   \
        ::ureact::detail::signal_node_ptr_t<right_val_t>>                                   \
>                                                                                   \
auto operator op(left_val_in_t&& lhs, const right_signal_t& rhs)                         \
    -> temp_signal<S,op_t>                                                            \
{                                                                                   \
    context* context = rhs.get_context();                                            \
    return temp_signal<S,op_t>(                                                       \
        context,                                                                    \
        std::make_shared<::ureact::detail::signal_op_node<S,op_t>>(                       \
            context, F( std::forward<left_val_in_t>(lhs) ), get_node_ptr(rhs)));         \
}                                                                                   \
template                                                                            \
<                                                                                   \
    typename left_val_t,                                                              \
    typename left_op_t,                                                               \
    typename right_val_t,                                                             \
    typename right_op_t,                                                              \
    typename F = ::ureact::detail::op_functors::op_functor_ ## name<left_val_t,right_val_t>,              \
    typename S = typename std::result_of<F(left_val_t,right_val_t)>::type,              \
    typename op_t = ::ureact::detail::function_op<S,F,left_op_t,right_op_t>                  \
>                                                                                   \
auto operator op(temp_signal<left_val_t,left_op_t>&& lhs,                                \
                 temp_signal<right_val_t,right_op_t>&& rhs)                              \
    -> temp_signal<S,op_t>                                                            \
{                                                                                   \
    context* context = lhs.get_context();                                            \
    assert(context == rhs.get_context());                                            \
    return temp_signal<S,op_t>(                                                       \
        context,                                                                    \
        std::make_shared<::ureact::detail::signal_op_node<S,op_t>>(                       \
            context, F(), lhs.steal_op(), rhs.steal_op()));                           \
}                                                                                   \
                                                                                    \
template                                                                            \
<                                                                                   \
    typename left_val_t,                                                              \
    typename left_op_t,                                                               \
    typename right_signal_t,                                                          \
    typename right_val_t = typename right_signal_t::value_t,                             \
    class = typename std::enable_if<                                                \
        is_signal<right_signal_t>::value>::type,                                       \
    typename F = ::ureact::detail::op_functors::op_functor_ ## name<left_val_t,right_val_t>,              \
    typename S = typename std::result_of<F(left_val_t,right_val_t)>::type,              \
    typename op_t = ::ureact::detail::function_op<S,F,                                   \
        left_op_t,                                                                    \
        ::ureact::detail::signal_node_ptr_t<right_val_t>>                                   \
>                                                                                   \
auto operator op(temp_signal<left_val_t,left_op_t>&& lhs,                                \
                 const right_signal_t& rhs)                                           \
    -> temp_signal<S,op_t>                                                            \
{                                                                                   \
    context* context = rhs.get_context();                                            \
    return temp_signal<S,op_t>(                                                       \
        context,                                                                    \
        std::make_shared<::ureact::detail::signal_op_node<S,op_t>>(                       \
            context, F(), lhs.steal_op(), get_node_ptr(rhs)));                         \
}                                                                                   \
                                                                                    \
template                                                                            \
<                                                                                   \
    typename left_signal_t,                                                           \
    typename right_val_t,                                                             \
    typename right_op_t,                                                              \
    typename left_val_t = typename left_signal_t::value_t,                               \
    class = typename std::enable_if<                                                \
        is_signal<left_signal_t>::value>::type,                                        \
    typename F = ::ureact::detail::op_functors::op_functor_ ## name<left_val_t,right_val_t>,              \
    typename S = typename std::result_of<F(left_val_t,right_val_t)>::type,              \
    typename op_t = ::ureact::detail::function_op<S,F,                                   \
        ::ureact::detail::signal_node_ptr_t<left_val_t>,                                    \
        right_op_t>                                                                   \
>                                                                                   \
auto operator op(const left_signal_t& lhs, temp_signal<right_val_t,right_op_t>&& rhs)      \
    -> temp_signal<S,op_t>                                                            \
{                                                                                   \
    context* context = lhs.get_context();                                            \
    return temp_signal<S,op_t>(                                                       \
        context,                                                                    \
        std::make_shared<::ureact::detail::signal_op_node<S,op_t>>(                       \
            context, F(), get_node_ptr(lhs), rhs.steal_op()));                         \
}                                                                                   \
                                                                                    \
template                                                                            \
<                                                                                   \
    typename left_val_t,                                                              \
    typename left_op_t,                                                               \
    typename right_val_in_t,                                                           \
    typename right_val_t = typename std::decay<right_val_in_t>::type,                    \
    class = typename std::enable_if<                                                \
        ! is_signal<right_val_t>::value>::type,                                        \
    typename F = ::ureact::detail::op_functors::op_l_functor_ ## name<left_val_t,right_val_t>,             \
    typename S = typename std::result_of<F(left_val_t)>::type,                        \
    typename op_t = ::ureact::detail::function_op<S,F,left_op_t>                           \
>                                                                                   \
auto operator op(temp_signal<left_val_t,left_op_t>&& lhs, right_val_in_t&& rhs)             \
    -> temp_signal<S,op_t>                                                            \
{                                                                                   \
    context* context = lhs.get_context();                                            \
    return temp_signal<S,op_t>(                                                       \
        context,                                                                    \
        std::make_shared<::ureact::detail::signal_op_node<S,op_t>>(                       \
            context, F( std::forward<right_val_in_t>(rhs) ), lhs.steal_op()));          \
}                                                                                   \
                                                                                    \
template                                                                            \
<                                                                                   \
    typename left_val_in_t,                                                            \
    typename right_val_t,                                                             \
    typename right_op_t,                                                              \
    typename left_val_t = typename std::decay<left_val_in_t>::type,                      \
    class = typename std::enable_if<                                                \
        ! is_signal<left_val_t>::value>::type,                                         \
    typename F = ::ureact::detail::op_functors::op_r_functor_ ## name<left_val_t,right_val_t>,             \
    typename S = typename std::result_of<F(right_val_t)>::type,                       \
    typename op_t = ::ureact::detail::function_op<S,F,right_op_t>                          \
>                                                                                   \
auto operator op(left_val_in_t&& lhs, temp_signal<right_val_t,right_op_t>&& rhs)            \
    -> temp_signal<S,op_t>                                                            \
{                                                                                   \
    context* context = rhs.get_context();                                            \
    return temp_signal<S,op_t>(                                                       \
        context,                                                                    \
        std::make_shared<::ureact::detail::signal_op_node<S,op_t>>(                       \
            context, F( std::forward<left_val_in_t>(lhs) ), rhs.steal_op()));           \
}                                                                                   \
}

REACT_DECLARE_OP(+, addition)
REACT_DECLARE_OP(-, subtraction)
REACT_DECLARE_OP(*, multiplication)
REACT_DECLARE_OP(/, division)
REACT_DECLARE_OP(%, modulo)

REACT_DECLARE_OP(==, equal)
REACT_DECLARE_OP(!=, not_equal)
REACT_DECLARE_OP(<,  less)
REACT_DECLARE_OP(<=, less_equal)
REACT_DECLARE_OP(>,  greater)
REACT_DECLARE_OP(>=, greater_equal)

REACT_DECLARE_OP(&&, logical_and)
REACT_DECLARE_OP(||, logical_or)

REACT_DECLARE_OP(&, bitwise_and)
REACT_DECLARE_OP(|, bitwise_or)
REACT_DECLARE_OP(^, bitwise_xor)
//REACT_DECLARE_OP(<<, bitwise_left_shift); // MSVC: Internal compiler error
//REACT_DECLARE_OP(>>, bitwise_right_shift);

#undef REACT_DECLARE_OP
