#pragma once

#include <utility>

namespace ureact
{

// Forward declaration to break cyclic dependency
template <typename S>
class signal;

namespace detail
{

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic push
#    pragma clang diagnostic ignored "-Wfloat-equal"
#endif

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

#if defined( __clang__ ) && defined( __clang_minor__ )
#    pragma clang diagnostic pop
#endif

} // namespace detail
} // namespace ureact
