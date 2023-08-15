//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_ZIP_HPP
#define UREACT_ADAPTOR_ZIP_HPP

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/zip_base.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct ZipAdaptor : Adaptor
{
    /*!
	 * @brief Emit a tuple (e1,…,eN) for each complete set of values for sources 1...N
	 *
	 *  Each source slot has its own unbounded buffer queue that persistently stores incoming events.
	 *  For as long as all queues are not empty, one value is popped from each and emitted together as a tuple.
	 *
	 *  Semantically equivalent of ranges::zip
	 */
    template <typename Source, typename... Sources>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<Source>& source1, const events<Sources>&... sources ) const
    {
        static_assert( sizeof...( Sources ) >= 1, "zip: 2+ arguments are required" );

        const context& context = source1.get_context();
        return detail::create_wrapped_node<events<std::tuple<Source, Sources...>>,
            event_zip_node<Source, Sources...>>( context, source1, sources... );
    }
};

} // namespace detail

inline constexpr detail::ZipAdaptor zip;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_ZIP_HPP
