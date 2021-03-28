#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

namespace ureact
{
namespace detail
{

template <typename T1, typename T2>
using is_same_decay = std::is_same<typename std::decay<T1>::type, typename std::decay<T2>::type>;

} // namespace detail
} // namespace ureact
