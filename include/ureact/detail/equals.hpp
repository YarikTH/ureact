#pragma once

#include <utility>

namespace ureact
{

// Forward declaration to break cyclic dependency
template <typename S>
class signal;

namespace detail
{

template <typename L, typename R>
bool equals( const L& lhs, const R& rhs )
{
    return lhs == rhs;
}

template <typename L, typename R>
bool equals( const std::reference_wrapper<L>& lhs, const std::reference_wrapper<R>& rhs )
{
    return lhs.get() == rhs.get();
}

template <typename L, typename R>
bool equals( const signal<L>& lhs, const signal<R>& rhs )
{
    return lhs.equals( rhs );
}

} // namespace detail
} // namespace ureact
