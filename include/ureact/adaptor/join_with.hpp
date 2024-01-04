//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_JOIN_WITH_HPP
#define UREACT_ADAPTOR_JOIN_WITH_HPP

#include <ureact/adaptor/process.hpp>
#include <ureact/detail/container_type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

struct JoinWithAdaptor : adaptor
{
    /*!
	 * @brief Emits the sequence obtained from flattening received event value, with the delimiter in between elements
     * 
     * The delimiter can be a single element or an iterable container of elements.
	 */
    template <typename InE, typename Pattern>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<InE>& source, Pattern&& pattern ) const
    {
        using E = container_value_t<InE>;
        return process<E>( source,                                     //
            [first = true, pattern = std::forward<Pattern>( pattern )] //
            ( event_range<InE> range, event_emitter<E> out ) mutable {
                if( !first )
                {
                    if constexpr( std::is_convertible_v<decltype( pattern ), E> )
                    {
                        out << pattern;
                    }
                    else if constexpr( std::is_convertible_v<container_value_t<decltype( pattern )>,
                                           E> )
                    {
                        for( const auto& i : pattern )
                            out << i;
                    }
                    else
                    {
                        static_assert( always_false<Pattern>, "Unsupported separator type" );
                    }
                }
                first = false;

                for( const auto& e : range )
                    for( const auto& i : e )
                        out << i;
            } );
    }

    template <typename Pattern>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( Pattern&& pattern ) const
    {
        return make_partial<JoinWithAdaptor>( std::forward<Pattern>( pattern ) );
    }
};

} // namespace detail

inline constexpr detail::JoinWithAdaptor join_with;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_JOIN_WITH_HPP
