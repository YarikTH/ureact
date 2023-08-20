//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <algorithm>
#include <cassert>
#include <string>

#include <catch2/catch_test_macros.hpp>
#include <nanobench.h>

#include "compiler_args.hpp"

TEST_CASE( "std headers include time" )
{
    using namespace std::chrono_literals;

    ankerl::nanobench::Bench bench;
    bench.title( "std headers include time" );
    bench.relative( true );
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

        perform_test( bench,
            std::string{ "baseline" } + " (" + compilerString + ')',
            CompilerArgs{ compilerArgs } //
                .source( "minimal.cpp" )
            //
        );

        for( const std::string& type : {
                 "",
                 "tuple",
                 "memory",
                 "vector",
                 "array",
                 "deque",
                 "functional",
                 "iterator",
                 "algorithm",
                 "all",
             } )
        {
            std::string name = "std";
            auto stdCompilerArgs = CompilerArgs{ compilerArgs } //
                                       .source( "include_std.cpp" );
            if( !type.empty() )
            {
                name += '+' + type;
                std::string definition = "INCLUDE_" + type;
                std::transform(
                    definition.begin(), definition.end(), definition.begin(), ::toupper );
                stdCompilerArgs.definition( definition );
            }

            perform_test( bench, name + " (" + compilerString + ')', stdCompilerArgs );
        }
    }
}
