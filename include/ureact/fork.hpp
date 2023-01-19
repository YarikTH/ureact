//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_FORK_HPP
#define UREACT_FORK_HPP

#include <tuple>

#include <ureact/closure.hpp>

UREACT_BEGIN_NAMESPACE

/*!
 * @brief Gets 1 or more functors (or closures) and invoke them all with the value from the left
 *
 *  Forward source so it can be chained further. Works similar to "tap" and "sink"
 */
template <typename... Destinations>
UREACT_WARN_UNUSED_RESULT auto fork( Destinations&&... destinations )
{
    static_assert( sizeof...( Destinations ) >= 1, "fork: at least 1 argument is required" );

    // TODO: propagate [[nodiscard(false)]] somehow
    return closure{
        [destinations = std::make_tuple( std::forward<Destinations>( destinations )... )] //
        ( const auto& source ) -> decltype( auto ) {
            /// call each passed function F(source)...
            std::apply(
                [&source]( const auto&... args ) { //
                    // TODO: remove it once nodiscard done right
                    static auto f = [&source]( const auto& f ) {
                        if constexpr( std::is_same_v<
                                          std::invoke_result_t<decltype( f ), decltype( source )>,
                                          void> )
                        {
                            f( source );
                        }
                        else
                        {
                            std::ignore = f( source );
                        }
                    };
                    ( f( args ), ... );
                    // TODO: use correct compact version after nodiscard done right
                    // ( args( source ), ... );
                },
                destinations );

            return source;
        } };
}

UREACT_END_NAMESPACE

#endif //UREACT_FORK_HPP
