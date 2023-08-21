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

namespace
{

enum class FunctionType
{
    unique,
    shared
};

enum class AdapterType
{
    pipe,
    func
};

} // namespace

TEST_CASE( "transform compilation time" )
{
    using namespace std::chrono_literals;

    ankerl::nanobench::Bench bench;
    bench.title( "transform compilation time" );
    //bench.relative( true );
    bench.performanceCounters( false );
    bench.timeUnit( 1ms, "ms" );
    bench.minEpochIterations( 1 );

    std::vector compilers = {
        Compiler{ gcc, 9 },
        //        Compiler{ gcc, 10 },
        //        Compiler{ gcc, 11 },
        Compiler{ gcc, 12 },
        Compiler{ clang, 11 },
        //        Compiler{ clang, 12 },
        //        Compiler{ clang, 13 },
        Compiler{ clang, 14 },
    };

    const auto args = generateCompilerArgs(
        compilers, { BuildConfiguration::Debug, BuildConfiguration::Release }, 17 );
    for( const auto& compilerArgs : args )
    {
        const auto compilerString = compilerArgs.get_name();

        [[maybe_unused]] const auto add_make_source_test
            = [&]( const int transformVersion, size_t i ) {
                  std::string name = "transform";
                  if( transformVersion > 1 )
                      name += std::to_string( transformVersion );

                  perform_test( bench,
                      std::string{ name + " make_source x" } + std::to_string( i ) + " ("
                          + compilerString + ')',
                      CompilerArgs{ compilerArgs } //
                          .source( "transform.cpp" )
                          .definition( "TRANSFORM_VERSION=" + std::to_string( transformVersion ) )
                          .definition( "MAKE_SOURCE_ONLY" )
                          .definition( std::string{ "COPY_COUNT=" } + std::to_string( i ) )
                      //
                  );
              };

        const auto add_transform_test = [&]( const int transformVersion,
                                            const size_t i,
                                            FunctionType funcType,
                                            AdapterType adapterType ) {
            std::string name = "transform";
            if( transformVersion > 1 )
                name += std::to_string( transformVersion );
            name += ' ';
            if( i > 1 )
            {
                name += funcType == FunctionType::unique ? "unique" : "shared";
                name += ' ';
            }
            name += adapterType == AdapterType::pipe ? "pipe" : "func";
            name += ' ';
            name += 'x' + std::to_string( i );

            perform_test( bench,
                name + " (" + compilerString + ')',
                CompilerArgs{ compilerArgs } //
                    .source( "transform.cpp" )
                    .definition( "TRANSFORM_VERSION=" + std::to_string( transformVersion ) )
                    .definition( std::string{ "UNIQUE_FUNC=" }
                                 + ( funcType == FunctionType::unique ? "true" : "false" ) )
                    .definition( std::string{ "USE_PIPE=" }
                                 + ( adapterType == AdapterType::pipe ? "true" : "false" ) )
                    .definition( std::string{ "COPY_COUNT=" } + std::to_string( i ) )
                //
            );
        };

        for( const auto& transformVersion : { 1, 2, 3, 4, 5 } )
        {
            //            {
            //                std::string name = "transform";
            //                if( transformVersion > 1 )
            //                    name += std::to_string( transformVersion );
            //                perform_test( bench,
            //                    std::string{ name + " include" } + " (" + compilerString + ')',
            //                    CompilerArgs{ compilerArgs } //
            //                        .source( "transform.cpp" )
            //                        .definition( "TRANSFORM_VERSION=" + std::to_string( transformVersion ) )
            //                        .definition( "INCLUDE_ONLY" )
            //                    //
            //                );
            //            }
            //
            //            add_make_source_test( transformVersion, 1 );
            //            add_make_source_test( transformVersion, 11 );

            //            add_transform_test( transformVersion, 1, FunctionType::unique, AdapterType::func );
            add_transform_test( transformVersion, 1, FunctionType::unique, AdapterType::pipe );
            //            add_transform_test( transformVersion, 11, FunctionType::shared, AdapterType::func );
            //            add_transform_test( transformVersion, 11, FunctionType::shared, AdapterType::pipe );
            //            add_transform_test( transformVersion, 11, FunctionType::unique, AdapterType::func );
            add_transform_test( transformVersion, 11, FunctionType::unique, AdapterType::pipe );
        }
    }
}
