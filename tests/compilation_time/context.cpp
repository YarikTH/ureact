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

TEST_CASE( "context compilation time" )
{
    using namespace std::chrono_literals;

    ankerl::nanobench::Bench bench;
    bench.title( "context compilation time" );
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
                    .source( "context.cpp" )
                    .definition( "INCLUDE_ONLY" )
                //
            );
        }

        {
            perform_test( bench,
                std::string{ "context" } + " (" + compilerString + ')',
                CompilerArgs{ compilerArgs } //
                    .source( "context.cpp" )
                //
            );
        }
    }
}
