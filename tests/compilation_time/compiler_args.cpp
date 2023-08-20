//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "compiler_args.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

#include <nanobench.h>

std::ostream& operator<<( std::ostream& os, const BuildConfiguration value )
{
    switch( value )
    {
        case BuildConfiguration::Default: os << "Default"; break;
        case BuildConfiguration::Release: os << "Release"; break;
        case BuildConfiguration::Debug: os << "Debug"; break;
    }
    return os;
}

std::ostream& operator<<( std::ostream& os, const CompilerName value )
{
    switch( value )
    {
        case CompilerName::clang: os << "clang++"; break;
        case CompilerName::gcc: os << "g++"; break;
    }
    return os;
}

std::ostream& operator<<( std::ostream& os, const Compiler value )
{
    return os << value.name << '-' << value.version;
}

std::string to_string( const Compiler value )
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

std::string CompilerArgs::build() const
{
    assert( isValid() );
    std::ostringstream ss;
    ss << m_compiler.value() << ' ';
    if( m_standard.has_value() )
        ss << "-std=" << m_standard.value() << ' ';
    switch( m_configuration.value_or( BuildConfiguration::Default ) )
    {
        case BuildConfiguration::Debug: ss << "-O0 -g" << ' '; break;
        case BuildConfiguration::Release: ss << "-O3 -DNDEBUG" << ' '; break;
        case BuildConfiguration::Default: break;
    }
    ss << "-I" << UREACT_INCLUDE_PATH << ' ';
    for( const auto& def : m_definitions )
        ss << "-D" << def << ' ';
    if( m_stdlib.has_value() )
        ss << "-stdlib=" << m_stdlib.value() << ' ';
    ss << "-c " << TEST_SAMPLES_PATH << '/' << m_source.value();
    return ss.str();
}

std::string CompilerArgs::get_name() const
{
    assert( m_compiler.has_value() );
    std::ostringstream ss;
    ss << m_compiler.value();
    if( m_standard )
        ss << ' ' << m_standard.value();
    if( m_stdlib )
        ss << ' ' << m_stdlib.value();
    if( m_configuration )
        ss << ' ' << m_configuration.value();
    return ss.str();
}

std::vector<CompilerArgs> generateCompilerArgs( const std::vector<Compiler>& compilers,
    const std::vector<BuildConfiguration>& configurations,
    const int minimalStandard )
{
    constexpr bool VERBOSE_ERRORS = false;

    std::vector<CompilerArgs> result;

    for( const auto& compiler : compilers )
    {
        if( std::system( ( CompilerArgs{} //
                               .compiler( to_string( compiler ) )
                               .source( "minimal.cpp" )
                               .build()
                           + " > /dev/null 2>&1" )
                             .c_str() )
            != 0 )
        {
            if constexpr( VERBOSE_ERRORS )
            {
                std::cerr << "skipping (" << to_string( compiler )
                          << ") because it can't compile the simple program" << std::endl;
            }
            continue;
        }

        const auto stdlibs = [compiler]() -> std::vector<std::string> {
            if( compiler.name == clang )
                return { "libstdc++", "libc++" };
            else
                return { "" };
        }();

        const auto standards = [compiler, minimalStandard]() -> std::vector<std::string> {
            std::vector<std::string> result;
            if( minimalStandard <= 11 )
                result.emplace_back( "c++11" );
            if( minimalStandard <= 14 )
                result.emplace_back( "c++14" );
            if( minimalStandard <= 17 )
                result.emplace_back( "c++17" );
            if( minimalStandard <= 20 )
            {
                if( compiler.name == gcc && compiler.version == 9 )
                    result.emplace_back( "c++2a" );
                else
                    result.emplace_back( "c++20" );
            }
            return result;
        }();

        for( const auto& configuration : configurations )
        {
            for( const auto& stdlib : stdlibs )
            {
                if( !stdlib.empty() )
                {
                    if( const auto args = CompilerArgs{} //
                                              .compiler( to_string( compiler ) )
                                              .stdlib( stdlib )
                                              .source( "minimal.cpp" );
                        std::system( ( args.build() + " > /dev/null 2>&1" ).c_str() ) != 0 )
                    {
                        if constexpr( VERBOSE_ERRORS )
                        {
                            std::cerr << "skipping (" << args.get_name()
                                      << ") because it can't compile the simple program"
                                      << std::endl;
                        }
                        continue;
                    }
                }

                for( const auto& standard : standards )
                {
                    auto compilerArgs = CompilerArgs{} //
                                            .compiler( to_string( compiler ) )
                                            .standard( standard )
                                            .configuration( configuration );
                    if( !stdlib.empty() )
                        compilerArgs.stdlib( stdlib );

                    if( std::system( ( CompilerArgs{ compilerArgs } //
                                           .source( "minimal.cpp" )
                                           .build()
                                       + " > /dev/null 2>&1" )
                                         .c_str() )
                        == 0 )
                    {
                        result.push_back( compilerArgs );
                    }
                    else
                    {
                        if constexpr( VERBOSE_ERRORS )
                        {
                            std::cerr << "skipping (" << compilerArgs.get_name()
                                      << ") because it can't compile the simple program"
                                      << std::endl;
                        }
                    }
                }
            }
        }
    }

    return result;
}

void perform_test(
    ankerl::nanobench::Bench& bench, const std::string& name, const CompilerArgs& compilerArgs )
{
    bench.run( name, [&] {
        std::system( compilerArgs.build().c_str() ); //
    } );
}
