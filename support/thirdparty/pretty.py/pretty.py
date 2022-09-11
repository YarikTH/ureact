#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ===============================================================================
import sys
import os
from enum import Enum
from typing import List
from dataclasses import dataclass

__version__ = '0.1.2'


@dataclass
class Tools:
    clang_format: str
    gersemi: str


# See https://stackoverflow.com/a/34177358
def is_tool(name):
    """Check whether `name` is on PATH."""

    from shutil import which

    return which(name) is not None


def check_tools(tools: Tools) -> bool:
    result: bool = True

    if not is_tool(tools.clang_format):
        result = False
        print(f"clang-format tool ({tools.clang_format}) is not found.\n"
              "You can install clang-format from apt:\n"
              "$ sudo apt install clang-format", file=sys.stderr)

    if not is_tool(tools.gersemi):
        result = False
        print(f"gersemi tool ({tools.gersemi}) is not found.\n"
              "You can install gersemi from PyPI:\n"
              "$ pip3 install gersemi", file=sys.stderr)

    return result


class FileType(Enum):
    UNKNOWN = -1
    CMAKE = 0
    CXX = 1


def detect_file_type(file: str) -> FileType:
    basename = os.path.basename(file)
    if basename == 'CMakeLists.txt':
        return FileType.CMAKE

    ext = os.path.splitext(basename)[-1]
    if ext in ['.cmake']:
        return FileType.CMAKE
    if ext in ['.c', '.cpp', '.h', '.hpp']:
        return FileType.CXX

    return FileType.UNKNOWN


def check_files(tools: Tools, cxx_files_to_check: List[str], cmake_files_to_check: List[str]) -> int:
    import subprocess

    result: int = 0

    if len(cxx_files_to_check) != 0:
        try:
            subprocess.check_call([tools.clang_format, '--Werror', '--dry-run'] + cxx_files_to_check)
        except subprocess.CalledProcessError:
            result = 1

    if len(cmake_files_to_check) != 0:
        try:
            subprocess.check_call([tools.gersemi, '--check'] + cmake_files_to_check)
        except subprocess.CalledProcessError:
            result = 1

    return result


def prettify_files(tools: Tools, cxx_files_to_check: List[str], cmake_files_to_check: List[str]) -> int:
    import subprocess

    result: int = 0

    if len(cxx_files_to_check) != 0:
        try:
            subprocess.check_call([tools.clang_format, '-i'] + cxx_files_to_check)
        except subprocess.CalledProcessError as e:
            result = e.returncode

    if len(cmake_files_to_check) != 0:
        try:
            subprocess.check_call([tools.gersemi, '--in-place', '-q'] + cmake_files_to_check)
        except subprocess.CalledProcessError as e:
            result = e.returncode

    return result


def process_files(tools: Tools, src: List[str], check: bool) -> int:
    # Batch files by types
    cxx_files_to_check = []
    cmake_files_to_check = []
    for f in src:
        file_type = detect_file_type(f)
        if file_type == FileType.CXX:
            cxx_files_to_check.append(f)
        elif file_type == FileType.CMAKE:
            cmake_files_to_check.append(f)
        else:
            pass  # TODO: add some kind of warning?

    if check:
        result = check_files(tools, cxx_files_to_check, cmake_files_to_check)

        if result == 0:
            print('Check is succeeded!')
        else:
            print('Check is failed! See the errors above.', file=sys.stderr)

        return result
    else:
        return prettify_files(tools, cxx_files_to_check, cmake_files_to_check)


def collect_non_ignored_files(working_tree_dir: str, excluded: List[str] = []) -> List[str]:
    import git

    file_entries: List[str] = []

    repo = git.Repo(working_tree_dir)

    excluded.extend([s.path for s in repo.submodules])

    excluded_paths = [os.path.join(working_tree_dir, x) for x in excluded]

    for root, dirs, files in os.walk(working_tree_dir):
        def is_dir_suitable(d: str) -> bool:
            full_path = os.path.join(root, d)
            if d == '.git':
                return False
            if full_path in excluded_paths:
                return False
            if len(repo.ignored(full_path)) != 0:
                return False
            return True

        def is_file_suitable(f: str) -> bool:
            full_path = os.path.join(root, f)
            if full_path in excluded_paths:
                return False
            if len(repo.ignored(full_path)) != 0:
                return False
            return True

        dirs[:] = [d for d in dirs if is_dir_suitable(d)]
        files[:] = [f for f in files if is_file_suitable(f)]

        file_entries.extend([os.path.relpath(os.path.join(root, f), working_tree_dir) for f in files])

    file_entries.sort()

    return file_entries


def detect_project_files(excluded: List[str] = []) -> List[str]:
    src = collect_non_ignored_files('.', excluded=excluded)
    src = [s for s in src if detect_file_type(s) != FileType.UNKNOWN]
    return src


def main() -> int:
    import argparse

    parser = argparse.ArgumentParser(description='A formatter to make your C++ and CMake code the real treasure.')
    parser.add_argument('src', type=str, nargs='*',
                        help='Files to format. Special value "-" to automatically detect project files')
    parser.add_argument('-c', '--check', action='store_true',
                        help='Check if files require reformatting. Return 0 when there\'s nothing to reformat, '
                             'return 1 when some files would be reformatted')
    parser.add_argument('--exclude', type=str, action='append',
                        help='Files and directories to exclude from formatting')
    parser.add_argument('--clang-format', type=str, default='clang-format',
                        help='Clang format to use for C++ files')
    parser.add_argument('--gersemi', type=str, default='gersemi',
                        help='Gersemi to use for CMake files')
    args = parser.parse_args()

    tools = Tools(args.clang_format, args.gersemi)
    if not check_tools(tools):
        return 2

    if args.src == ["-"]:
        exclude = [] if args.exclude is None else args.exclude
        src = detect_project_files(exclude)
    else:
        src = args.src
    src.sort()

    return process_files(tools=tools, src=src, check=args.check)


# Required packages:
# * GitPython	3.1.24
if __name__ == '__main__':
    status = main()
    sys.exit(status)
