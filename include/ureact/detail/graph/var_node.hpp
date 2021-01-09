#pragma once

#include <cassert>
#include "ureact/detail/graph/signal_node.hpp"

namespace ureact { namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// var_node
///////////////////////////////////////////////////////////////////////////////////////////////////
template
<
    typename S
>
class var_node :
    public signal_node<S>,
    public i_input_node
{
public:
    template <typename T>
    explicit var_node(context* context, T&& value) :
        var_node::signal_node( context, std::forward<T>(value) ),
        new_value_( value )
    {
        var_node::signal_node::get_context()->on_node_create(*this);
    }
    
    var_node(const var_node&) = delete;
    var_node& operator=(const var_node&) = delete;
    var_node(var_node&&) noexcept = delete;
    var_node& operator=(var_node&&) noexcept = delete;
    
    ~var_node() override
    {
        var_node::signal_node::get_context()->on_node_destroy(*this);
    }

    const char* get_node_type() const override        { return "var_node"; }

    void tick(turn_base& /*turn*/) override
    {
        assert(false && "Ticked var_node");
    }

    template <typename V>
    void add_input(V&& new_value)
    {
        new_value_ = std::forward<V>(new_value);

        is_input_added_ = true;

        // is_input_added_ takes precedences over is_input_modified_
        // the only difference between the two is that is_input_modified_ doesn't/can't compare
        is_input_modified_ = false;
    }

    // This is signal-specific
    template <typename F>
    void modify_input(F& func)
    {
        // There hasn't been any set(...) input yet, modify.
        if (! is_input_added_)
        {
            func(this->value_);

            is_input_modified_ = true;
        }
        // There's a new_value, modify new_value instead.
        // The modified new_value will handled like before, i.e. it'll be compared to value_
        // in apply_input
        else
        {
            func(new_value_);
        }
    }

    bool apply_input(turn_base& turn) override
    {
        if (is_input_added_)
        {
            is_input_added_ = false;

            if (! equals(this->value_, new_value_))
            {
                this->value_ = std::move(new_value_);
                var_node::get_context()->on_input_change(*this, turn);
                return true;
            }
            return false;
        }
        if (is_input_modified_)
        {
            is_input_modified_ = false;

            var_node::get_context()->on_input_change(*this, turn);
            return true;
        }
        return false;
    }

private:
    S       new_value_;
    bool    is_input_added_ = false;
    bool    is_input_modified_ = false;
};

}}
