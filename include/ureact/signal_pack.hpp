#pragma once

//#include "ureact/signal_.hpp"

namespace ureact
{

template <typename S>
class signal;

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal_pack - Wraps several nodes in a tuple. Create with comma operator.
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... values_t>
class signal_pack
{
public:
    explicit signal_pack( const signal<values_t>&... deps )
        : data( std::tie( deps... ) )
    {}

    template <typename... cur_values_t, typename append_value_t>
    signal_pack( const signal_pack<cur_values_t...>& cur_args, const signal<append_value_t>& new_arg )
        : data( std::tuple_cat( cur_args.data, std::tie( new_arg ) ) )
    {}

    std::tuple<const signal<values_t>&...> data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// with - Utility function to create a signal_pack
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... values_t>
auto with( const signal<values_t>&... deps ) -> signal_pack<values_t...>
{
    return signal_pack<values_t...>( deps... );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Comma operator overload to create signal pack from 2 signals.
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename left_val_t, typename right_val_t>
auto operator,( const signal<left_val_t>& a, const signal<right_val_t>& b )
    -> signal_pack<left_val_t, right_val_t>
{
    return signal_pack<left_val_t, right_val_t>( a, b );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Comma operator overload to append node to existing signal pack.
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... cur_values_t, typename append_value_t>
auto operator,( const signal_pack<cur_values_t...>& cur, const signal<append_value_t>& append )
    -> signal_pack<cur_values_t..., append_value_t>
{
    return signal_pack<cur_values_t..., append_value_t>( cur, append );
}

}
