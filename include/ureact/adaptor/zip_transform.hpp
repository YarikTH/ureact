//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_ZIP_TRANSFORM_HPP
#define UREACT_ADAPTOR_ZIP_TRANSFORM_HPP

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/zip_base.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct ZipTransformAdaptor : Adaptor
{
    /*!
	 * @brief Emit an invoke result for func(e1,…,eN) for each complete set of values for sources 1...N
	 *
	 *  Each source slot has its own unbounded buffer queue that persistently stores incoming events.
	 *  For as long as all queues are not empty, one value is popped from each and emitted together as a tuple.
	 *
	 *  Semantically equivalent of ranges::zip_transform
	 */
    template <typename F, typename Source, typename... Sources>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        F&& func, const events<Source>& source1, const events<Sources>&... sources ) const
    {
        static_assert( sizeof...( Sources ) >= 1, "zip: 2+ arguments are required" );

        using node_type = event_zip_node<F, Source, Sources...>;
        using E = typename node_type::E;

        const context& context = source1.get_context();
        return detail::create_wrapped_node<events<E>, node_type>(
            context, std::forward<F>( func ), source1, sources... );
    }
};

} // namespace detail

inline constexpr detail::ZipTransformAdaptor zip_transform;

UREACT_END_NAMESPACE

#endif //UREACT_ADAPTOR_ZIP_TRANSFORM_HPP
