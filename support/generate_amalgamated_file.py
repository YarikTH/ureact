#!/usr/bin/env python3

# Script to amalgamate all ureact headers into single header
# Inspired by https://github.com/catchorg/Catch2/blob/v3.1.0/tools/scripts/generateAmalgamatedFiles.py

import os
import re
import datetime
from pathlib import Path
from typing import List, Set

# Path to support folder
support_folder_path = Path(__file__).parent.absolute()
assert support_folder_path.name == 'support', f'Assuming {Path(__file__).name} is placed in support folder'

# Path to ureact repository
root_path = support_folder_path.parent

# Path to ureact headers
ureact_include_path = root_path.joinpath('include')

# Path to result header
output_header = root_path.joinpath(root_path, 'single_include', 'ureact', 'ureact_amalgamated.hpp')

# Compiled regular expression to detect ureact self includes
internal_include_parser = re.compile(r'\s*#\s*include <(ureact/.*)>.*')

# Copyright text
copyright_text = '''\
//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Yaroslav Krylov.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
'''

ureact_introduction_text = '''\
// ureact - C++ header-only FRP library
// The library is heavily influenced by cpp.react - https://github.com/snakster/cpp.react
// which uses the Boost Software License - Version 1.0
// see here - https://github.com/snakster/cpp.react/blob/master/LICENSE_1_0.txt
// Project started as a complex refactoring and transformation of cpp.react's codebase
// but with different design goals.
// The documentation can be found at the library's page:
// https://github.com/YarikTH/ureact/blob/main/README.md
'''

# These are the copyright comments in each file, we want to ignore them
ignored_lines = (copyright_text + ureact_introduction_text).splitlines(keepends=True)

# The header of the amalgamated file: copyright information + explanation
# what this file is.
file_header = '''\
// =============================================================
// == DO NOT MODIFY THIS FILE BY HAND - IT IS AUTO GENERATED! ==
// =============================================================
{copyright_text}\
// ----------------------------------------------------------------
// Ureact v{version_string}
// Generated: {generation_time}
// ----------------------------------------------------------------
{ureact_introduction_text}\
// ----------------------------------------------------------------
// This file is an amalgamation of multiple different files.
// You probably shouldn't edit it directly.
// It is mostly intended to be used from godbolt and similar tools.
// ----------------------------------------------------------------
'''


def detect_version_string() -> str:
    version_parser = re.compile(r'#define UREACT_VERSION_STR "([^"]+)".*')

    with open(Path(ureact_include_path, 'ureact', 'version.hpp'), mode='r', encoding='utf-8') as header:
        for line in header:
            m = version_parser.match(line)
            if m:
                return m.group(1)

    raise Exception("Can't detect ureact version!")


def formatted_file_header() -> str:
    # Returns file header with proper version string and generation time
    return file_header.format(copyright_text=copyright_text,
                              ureact_introduction_text=ureact_introduction_text,
                              version_string=detect_version_string(),
                              generation_time=datetime.datetime.now())


def discover_hpp(where: Path) -> List[Path]:
    result: List[Path] = []

    for root, dirs, filenames in os.walk(where):
        dirs[:] = [d for d in dirs if d != 'detail']
        for filename in [Path(f) for f in filenames]:
            if filename.suffix == '.hpp':
            # if filename.name in ['events.hpp']: # events fwd
            # if filename.name in ['signal.hpp']: # signal fwd
            # if filename.name in ['observer.hpp']: # observer fwd
            # if filename.name in ['take_drop.hpp', 'take_drop_while.hpp', 'unique.hpp', 'pulse.hpp', 'zip.hpp', 'merge.hpp', 'fork.hpp', 'sink.hpp', 'unify.hpp', 'flatten.hpp', 'tap.hpp']: # events all
            # if filename.name in ['fork.hpp', 'sink.hpp', 'lift.hpp', 'flatten.hpp', 'tap.hpp']: # signal all
                result.append(Path(root, filename).relative_to(where))

    result.sort()

    result = [Path('ureact', 'version.hpp')] + result

    return result


def amalgamate_source_file(out, filename: Path, /, processed_headers: Set[Path], expand_headers: bool) -> int:
    # print(f'-- processing {filename.relative_to(ureact_include_path)}')

    # Gathers statistics on how many headers were expanded
    concatenated = 1

    define_guard_name = re.sub('[^0-9a-zA-Z]+', '_', str(filename.relative_to(ureact_include_path)).upper())

    define_guard_lines = [
        f'#ifndef {define_guard_name}\n',
        f'#define {define_guard_name}\n',
        f'#endif // {define_guard_name}\n',
    ]

    with open(filename, mode='r', encoding='utf-8') as input:
        for line in input:
            # ignore copyright and description lines
            if line in ignored_lines:
                continue

            # I prefer not to ignore them, because it allows to mix up amalgamated
            # header with normal headers
            # ignore define guard lines
            # if line in define_guard_lines:
            #     continue

            m = internal_include_parser.match(line)

            # anything that isn't a ureact header can just be copied to
            # the resulting file
            if not m:
                out.write(line)
                continue

            # We do not want to expand headers for the cpp file amalgamation
            # but neither do we want to copy them to output
            if not expand_headers:
                continue

            next_header = Path(m.group(1))

            # We have to avoid re-expanding the same header over and over again
            if next_header in processed_headers:
                continue

            processed_headers.add(next_header)
            concatenated += amalgamate_source_file(out, Path(ureact_include_path, next_header),
                                                   processed_headers=processed_headers,  #
                                                   expand_headers=expand_headers)

    return concatenated


def amalgamate_sources(out, files: List[Path], /, processed_headers: Set[Path], expand_headers: bool) -> int:
    # Gathers statistics on how many headers were expanded
    concatenated = 0

    for next_header in files:
        if next_header not in processed_headers:
            processed_headers.add(next_header)
            concatenated += amalgamate_source_file(out, Path(ureact_include_path, next_header),
                                                   processed_headers=processed_headers,  #
                                                   expand_headers=expand_headers)

    return concatenated


def generate_header():
    output_header.parent.mkdir(exist_ok=True, parents=True)

    with open(output_header, mode='w', encoding='utf-8') as header:
        header.write(formatted_file_header())
        header.write('#ifndef UREACT_UREACT_AMALGAMATED_HPP\n')
        header.write('#define UREACT_UREACT_AMALGAMATED_HPP\n')
        concatenated = amalgamate_sources(header, discover_hpp(ureact_include_path), processed_headers=set(),
                                          expand_headers=True)
        # print(f'Concatenated {concatenated} headers')
        header.write('#endif // UREACT_UREACT_AMALGAMATED_HPP\n')


if __name__ == "__main__":
    generate_header()
