//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

enum class BuildConfiguration
{
    Default,
    Release,
    Debug
};

std::ostream& operator<<( std::ostream& os, const BuildConfiguration value );

enum class CompilerName
{
    clang,
    gcc
};

constexpr inline auto clang = CompilerName::clang;
constexpr inline auto gcc = CompilerName::gcc;

std::ostream& operator<<( std::ostream& os, const CompilerName value );

struct Compiler
{
    CompilerName name;
    int version;
};

std::ostream& operator<<( std::ostream& os, const Compiler value );

std::string to_string( const Compiler value );

class CompilerArgs
{
public:
    CompilerArgs& compiler( const std::string& value )
    {
        m_compiler = value;
        return *this;
    }

    CompilerArgs& standard( const std::string& value )
    {
        m_standard = value;
        return *this;
    }

    CompilerArgs& configuration( const BuildConfiguration value )
    {
        m_configuration = value;
        return *this;
    }

    CompilerArgs& source( const std::string& value )
    {
        m_source = value;
        return *this;
    }

    CompilerArgs& definition( const std::string& value )
    {
        m_definitions.push_back( value );
        return *this;
    }

    CompilerArgs& stdlib( const std::string& value )
    {
        m_stdlib = value;
        return *this;
    }

    [[nodiscard]] bool isValid() const
    {
        return m_compiler.has_value() && m_source.has_value();
    }

    [[nodiscard]] std::string build() const;

    [[nodiscard]] std::string get_name() const;

private:
    std::optional<std::string> m_compiler;
    std::optional<std::string> m_standard;
    std::optional<BuildConfiguration> m_configuration;
    std::vector<std::string> m_definitions;
    std::optional<std::string> m_stdlib;
    std::optional<std::string> m_source;
};

std::vector<CompilerArgs> generateCompilerArgs( const std::vector<Compiler>& compilers,
    const std::vector<BuildConfiguration>& configurations,
    const int minimalStandard );

namespace ankerl::nanobench
{
class Bench;
}

void perform_test(
    ankerl::nanobench::Bench& bench, const std::string& name, const CompilerArgs& compilerArgs );
