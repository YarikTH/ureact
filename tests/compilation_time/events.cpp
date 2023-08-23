//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <catch2/catch_test_macros.hpp>
#include <nanobench.h>

#include "compiler_args.hpp"

TEST_CASE( "events compilation time" )
{
    using namespace std::chrono_literals;

    ankerl::nanobench::Bench bench;
    bench.title( "events compilation time" );
    //bench.relative( true );
    bench.performanceCounters( false );
    bench.timeUnit( 1ms, "ms" );
    bench.minEpochIterations( 1 );

    std::vector compilers = {
        Compiler{ gcc, 9 },
        Compiler{ gcc, 10 },
        Compiler{ gcc, 11 },
        Compiler{ gcc, 12 },
        Compiler{ clang, 11 },
        Compiler{ clang, 12 },
        Compiler{ clang, 13 },
        Compiler{ clang, 14 },
    };

    const auto args = generateCompilerArgs(
        compilers, { BuildConfiguration::Debug, BuildConfiguration::Release }, 17 );
    for( const auto& compilerArgs : args )
    {
        const auto compilerString = compilerArgs.get_name();

        [[maybe_unused]] const auto add_member_events_test = [&]( const bool useMemberEvents,
                                                                 const bool useDifferentTypes,
                                                                 const bool initialize,
                                                                 const size_t i ) {
            std::string name = useMemberEvents ? "member_events" : "events";
            if( i > 1 )
            {
                name += ' ';
                name += useDifferentTypes ? "different" : "same";
            }
            if( initialize )
            {
                name += ' ';
                name += "init";
            }
            name += ' ';
            name += 'x' + std::to_string( i );

            perform_test( bench,
                name + " (" + compilerString + ')',
                CompilerArgs{ compilerArgs } //
                    .source( "events.cpp" )
                    .definition(
                        std::string{ "USE_MEMBER_EVENTS=" } + ( useMemberEvents ? "1" : "0" ) )
                    .definition(
                        std::string{ "USE_DIFFERENT_TYPES=" } + ( useDifferentTypes ? "1" : "0" ) )
                    .definition( std::string{ "INITIALIZE_EVENTS=" } + ( initialize ? "1" : "0" ) )
                    .definition( std::string{ "COPY_COUNT=" } + std::to_string( i ) )
                //
            );
        };

        {
            perform_test( bench,
                std::string{ "std include" } + " (" + compilerString + ')',
                CompilerArgs{ compilerArgs } //
                    .source( "include_std.cpp" )
                    .definition( "INCLUDE_ALL" )
                //
            );
        }

        {
            perform_test( bench,
                std::string{ "include" } + " (" + compilerString + ')',
                CompilerArgs{ compilerArgs } //
                    .source( "events.cpp" )
                    .definition( "INCLUDE_ONLY" )
                //
            );
        }

        for( const auto& initialize : { false, true } )
        {
            for( const auto& useMemberEvents : { false, true } )
            {
                add_member_events_test(
                    useMemberEvents, false /*useDifferentTypes*/, initialize, 1 );
                add_member_events_test(
                    useMemberEvents, false /*useDifferentTypes*/, initialize, 11 );
                add_member_events_test(
                    useMemberEvents, true /*useDifferentTypes*/, initialize, 11 );
            }
        }
    }
}
