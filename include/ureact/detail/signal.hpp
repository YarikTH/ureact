#pragma once

#include "ureact/detail/signal_base.hpp"
#include "ureact/detail/flatten.hpp"
#include "ureact/detail/type_traits.hpp"

namespace ureact
{

///////////////////////////////////////////////////////////////////////////////////////////////////
/// signal
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class signal : public detail::signal_base<S>
{
private:
    using node_t = detail::signal_node<S>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
    using value_t = S;

    signal() = default;

    explicit signal( node_ptr_t&& node_ptr )
        : signal::signal_base( std::move( node_ptr ) )
    {}

    const S& value() const
    {
        return signal::signal_base::get_value();
    }

    S flatten() const
    {
        static_assert( is_signal<S>::value, "flatten requires a signal value type." );
        return ::ureact::flatten( *this );
    }
};

// Specialize for references
template <typename S>
class signal<S&> : public detail::signal_base<std::reference_wrapper<S>>
{
private:
    using node_t = detail::signal_node<std::reference_wrapper<S>>;
    using node_ptr_t = std::shared_ptr<node_t>;

public:
    using value_t = S;

    signal() = default;

    explicit signal( node_ptr_t&& node_ptr )
        : signal::signal_base( std::move( node_ptr ) )
    {}

    const S& value() const
    {
        return signal::signal_base::get_value();
    }
};

} // namespace ureact
