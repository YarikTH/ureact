#
#         Copyright (C) 2020-2021 Krylov Yaroslav.
#
# Distributed under the Boost Software License, Version 1.0.
#    (See accompanying file LICENSE_1_0.txt or copy at
#          http://www.boost.org/LICENSE_1_0.txt)
#
# Based on the work of Alexey Ochapov (https://github.com/alexezeder)
#
function(set_gcc_warning_flags result_var_name)
    set(GCC_WARNING_OPTIONS)

    # -Waddress
    # -Warray-bounds=1 (only with -O2)
    # -Wbool-compare
    # -Wbool-operation
    # -Wc++11-compat  -Wc++14-compat
    # -Wcatch-value (C++ and Objective-C++ only)
    # -Wchar-subscripts
    # -Wcomment
    # -Wduplicate-decl-specifier (C and Objective-C only)
    # -Wenum-compare (in C/ObjC; this is on by default in C++)
    # -Wenum-conversion in C/ObjC;
    # -Wformat
    # -Wformat-overflow
    # -Wformat-truncation
    # -Wint-in-bool-context
    # -Wimplicit (C and Objective-C only)
    # -Wimplicit-int (C and Objective-C only)
    # -Wimplicit-function-declaration (C and Objective-C only)
    # -Winit-self (only for C++)
    # -Wlogical-not-parentheses
    # -Wmain (only for C/ObjC and unless -ffreestanding)
    # -Wmaybe-uninitialized
    # -Wmemset-elt-size
    # -Wmemset-transposed-args
    # -Wmisleading-indentation (only for C/C++)
    # -Wmissing-attributes
    # -Wmissing-braces (only for C/ObjC)
    # -Wmultistatement-macros
    # -Wnarrowing (only for C++)
    # -Wnonnull
    # -Wnonnull-compare
    # -Wopenmp-simd
    # -Wparentheses
    # -Wpessimizing-move (only for C++)
    # -Wpointer-sign
    # -Wreorder
    # -Wrestrict
    # -Wreturn-type
    # -Wsequence-point
    # -Wsign-compare (only in C++)
    # -Wsizeof-pointer-div
    # -Wsizeof-pointer-memaccess
    # -Wstrict-aliasing
    # -Wstrict-overflow=1
    # -Wswitch
    # -Wtautological-compare
    # -Wtrigraphs
    # -Wuninitialized
    # -Wunknown-pragmas
    # -Wunused-function
    # -Wunused-label
    # -Wunused-value
    # -Wunused-variable
    # -Wvolatile-register-var
    # -Wzero-length-bounds
    list(APPEND GCC_WARNING_OPTIONS "-Wall")

    # -Wclobbered
    # -Wcast-function-type
    # -Wdeprecated-copy (C++ only)
    # -Wempty-body
    # -Wignored-qualifiers
    # -Wimplicit-fallthrough=3
    # -Wmissing-field-initializers
    # -Wmissing-parameter-type (C only)
    # -Wold-style-declaration (C only)
    # -Woverride-init
    # -Wsign-compare (C only)
    # -Wstring-compare
    # -Wredundant-move (only for C++)
    # -Wtype-limits
    # -Wuninitialized
    # -Wshift-negative-value (in C++03 and in C99 and newer)
    # -Wunused-parameter (only with -Wunused or -Wall)
    # -Wunused-but-set-parameter (only with -Wunused or -Wall)
    list(APPEND GCC_WARNING_OPTIONS "-Wextra")

    # Issue all the warnings demanded by strict ISO C and ISO C++; reject all
    # programs that use forbidden extensions, and some other programs that do
    # not follow ISO C and ISO C++. For ISO C, follows the version of the ISO
    # C standard specified by any -std option used.
    list(APPEND GCC_WARNING_OPTIONS "-Wpedantic")

    # Check calls to printf and scanf, etc., to make sure that the arguments supplied
    # have types appropriate to the format string specified, and that the conversions
    # specified in the format string make sense. This includes standard functions,
    # and others specified by format attributes (see Function Attributes), in
    # the printf, scanf, strftime and strfmon (an X/Open extension, not in
    # the C standard) families (or other target-specific families).
    # Which functions are checked without format attributes having been specified
    # depends on the standard version selected, and such checks of functions without
    # the attribute specified are disabled by -ffreestanding or -fno-builtin.
    # The formats are checked against the format features supported by GNU libc
    # version 2.2. These include all ISO C90 and C99 features, as well as features
    # from the Single Unix Specification and some BSD and GNU extensions.
    # Other library implementations may not support all these features;
    # GCC does not support warning about features that go beyond a particular library's
    # limitations. However, if -Wpedantic is used with -Wformat, warnings are
    # given about format features not in the selected standard version
    # (but not for strfmon formats, since those are not in any version of
    # the C standard). See Options Controlling C Dialect.
    # Enable -Wformat plus additional format checks. Currently equivalent to
    # -Wformat -Wformat-nonliteral -Wformat-security -Wformat-y2k.
    list(APPEND GCC_WARNING_OPTIONS "-Wformat=2")

    # Warn if the compiler detects paths that trigger erroneous or undefined
    # behavior due to dereferencing a null pointer.
    # This option is only active when -fdelete-null-pointer-checks is active,
    # which is enabled by optimizations in most targets. The precision of
    # the warnings depends on the optimization options used.
    list(APPEND GCC_WARNING_OPTIONS "-Wnull-dereference")

    # Warn about uninitialized variables that are initialized with themselves.
    # Note this option can only be used with the -Wuninitialized option.
    list(APPEND GCC_WARNING_OPTIONS "-Winit-self")

    # C++17 provides a standard way to suppress the -Wimplicit-fallthrough warning
    # using [[fallthrough]]
    # -Wimplicit-fallthrough=5 doesn't recognize any comments as fallthrough comments,
    # only attributes disable the warning.
    list(APPEND GCC_WARNING_OPTIONS "-Wimplicit-fallthrough=5")

    list(APPEND GCC_WARNING_OPTIONS "-Wunused")
    list(APPEND GCC_WARNING_OPTIONS "-Wno-unused-function")
    list(APPEND GCC_WARNING_OPTIONS "-Wno-unused-parameter")

    # Warn if an automatic variable is used without first being initialized.
    # In C++, warn if a non-static reference or non-static const member appears
    # in a class without constructors.
    list(APPEND GCC_WARNING_OPTIONS "-Wuninitialized")
    list(APPEND GCC_WARNING_OPTIONS "-Wmaybe-uninitialized")

    list(APPEND GCC_WARNING_OPTIONS "-Wsuggest-attribute=noreturn")

    # Warn about duplicated conditions in an if-else-if chain.
    list(APPEND GCC_WARNING_OPTIONS "-Wduplicated-cond")

    # Warn if an undefined identifier is evaluated in an #if directive.
    # Such identifiers are replaced with zero.
    list(APPEND GCC_WARNING_OPTIONS "-Wundef")

    # Warn whenever a pointer is cast such that the required alignment of the target
    # is increased. For example, warn if a char * is cast to an int * on machines
    # where integers can only be accessed at two- or four-byte boundaries.
    list(APPEND GCC_WARNING_OPTIONS "-Wcast-align")

    # Warn when macros __TIME__, __DATE__ or __TIMESTAMP__ are encountered as they
    # might prevent bit-wise-identical reproducible compilations.
    list(APPEND GCC_WARNING_OPTIONS "-Wdate-time")

    # Warn about suspicious uses of logical operators in expressions.
    # This includes using logical operators in contexts where a bit-wise operator
    # is likely to be expected.
    list(APPEND GCC_WARNING_OPTIONS "-Wlogical-op")

    # Warn if a variable-length array is used in the code.
    list(APPEND GCC_WARNING_OPTIONS "-Wvla")

    list(APPEND GCC_WARNING_OPTIONS "-Wdelete-non-virtual-dtor")

    list(APPEND GCC_WARNING_OPTIONS "-Wno-error=null-dereference")

    set(${result_var_name} "${GCC_WARNING_OPTIONS}" PARENT_SCOPE)
endfunction()

function(set_clang_warning_flags result_var_name)
    set(CLANG_WARNING_OPTIONS)

    # -Wmisleading-indentation
    # -Wmost
    # -Wparentheses
    # -Wswitch
    # -Wswitch-bool
    list(APPEND CLANG_WARNING_OPTIONS "-Wall")

    list(APPEND CLANG_WARNING_OPTIONS "-Wextra")

    list(APPEND CLANG_WARNING_OPTIONS "-Wpedantic")

    list(APPEND CLANG_WARNING_OPTIONS "-Wformat=2")

    list(APPEND CLANG_WARNING_OPTIONS "-Wnull-dereference")

    list(APPEND CLANG_WARNING_OPTIONS "-Wimplicit-fallthrough")

    list(APPEND CLANG_WARNING_OPTIONS "-Wunused")
    list(APPEND CLANG_WARNING_OPTIONS "-Wno-unused-function")
    list(APPEND CLANG_WARNING_OPTIONS "-Wno-unused-parameter")

    list(APPEND CLANG_WARNING_OPTIONS "-Wuninitialized")
    list(APPEND CLANG_WARNING_OPTIONS "-Wconditional-uninitialized")

    list(APPEND CLANG_WARNING_OPTIONS "-Wmissing-noreturn")

    list(APPEND CLANG_WARNING_OPTIONS "-Wundef")

    list(APPEND CLANG_WARNING_OPTIONS "-Wcast-align")

    list(APPEND CLANG_WARNING_OPTIONS "-Wdate-time")

    list(APPEND CLANG_WARNING_OPTIONS "-Wvla")

    list(APPEND CLANG_WARNING_OPTIONS "-Wdelete-non-virtual-dtor")

    list(APPEND CLANG_WARNING_OPTIONS "-Wassign-enum")
    list(APPEND CLANG_WARNING_OPTIONS "-Wbad-function-cast")
    list(APPEND CLANG_WARNING_OPTIONS "-Wcast-qual")
    list(APPEND CLANG_WARNING_OPTIONS "-Wcomma")

    list(APPEND CLANG_WARNING_OPTIONS "-Wconversion")
    list(APPEND CLANG_WARNING_OPTIONS "-Wno-sign-conversion")
    list(APPEND CLANG_WARNING_OPTIONS "-Wno-shorten-64-to-32")

    list(APPEND CLANG_WARNING_OPTIONS "-Wdeprecated")
    list(APPEND CLANG_WARNING_OPTIONS "-Wduplicate-enum")
    list(APPEND CLANG_WARNING_OPTIONS "-Wempty-init-stmt")
    list(APPEND CLANG_WARNING_OPTIONS "-Wexplicit-initialize-call")
    #    list(APPEND CLANG_WARNING_OPTIONS "-Wextra-semi")
    list(APPEND CLANG_WARNING_OPTIONS "-Wextra-semi-stmt")
    list(APPEND CLANG_WARNING_OPTIONS "-Wfloat-equal")
    list(APPEND CLANG_WARNING_OPTIONS "-Wformat-pedantic")
    list(APPEND CLANG_WARNING_OPTIONS "-Wfour-char-constants")
    list(APPEND CLANG_WARNING_OPTIONS "-Wgcc-compat")
    list(APPEND CLANG_WARNING_OPTIONS "-Wgnu")
    list(APPEND CLANG_WARNING_OPTIONS "-Wheader-hygiene")
    list(APPEND CLANG_WARNING_OPTIONS "-Widiomatic-parentheses")
    list(APPEND CLANG_WARNING_OPTIONS "-Wimplicit")
    list(
        APPEND
        CLANG_WARNING_OPTIONS
        "-Winconsistent-missing-destructor-override"
    )
    list(APPEND CLANG_WARNING_OPTIONS "-Wkeyword-macro")
    list(APPEND CLANG_WARNING_OPTIONS "-Wmethod-signatures")
    list(APPEND CLANG_WARNING_OPTIONS "-Wnewline-eof")
    list(APPEND CLANG_WARNING_OPTIONS "-Wold-style-cast")
    list(APPEND CLANG_WARNING_OPTIONS "-Woverriding-method-mismatch")
    #    list(APPEND CLANG_WARNING_OPTIONS "-Wreserved-id-macro")
    list(APPEND CLANG_WARNING_OPTIONS "-Wreserved-user-defined-literal")
    list(APPEND CLANG_WARNING_OPTIONS "-Wtautological-compare")
    list(APPEND CLANG_WARNING_OPTIONS "-Wundefined-func-template")
    list(APPEND CLANG_WARNING_OPTIONS "-Wundefined-reinterpret-cast")
    list(APPEND CLANG_WARNING_OPTIONS "-Wunreachable-code-aggressive")
    list(APPEND CLANG_WARNING_OPTIONS "-Wvariadic-macros")
    list(APPEND CLANG_WARNING_OPTIONS "-Wvector-conversion")

    list(APPEND CLANG_WARNING_OPTIONS "-Wno-error=float-equal")
    list(APPEND CLANG_WARNING_OPTIONS "-Wno-error=float-conversion")
    list(APPEND CLANG_WARNING_OPTIONS "-Wno-error=implicit-float-conversion")
    list(APPEND CLANG_WARNING_OPTIONS "-Wno-error=unused-private-field")
    list(APPEND CLANG_WARNING_OPTIONS "-Wno-error=undefined-func-template")

    set(${result_var_name} "${CLANG_WARNING_OPTIONS}" PARENT_SCOPE)
endfunction()

function(set_msvc_warning_flags result_var_name)
    set(MSVC_WARNING_OPTIONS)

    list(APPEND MSVC_WARNING_OPTIONS /W3)

    set(${result_var_name} "${MSVC_WARNING_OPTIONS}" PARENT_SCOPE)
endfunction()

function(set_warning_flags result_var_name)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set_gcc_warning_flags(WARNING_OPTIONS)
    elseif(
        CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"
    )
        set_clang_warning_flags(WARNING_OPTIONS)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set_msvc_warning_flags(WARNING_OPTIONS)
    else()
        message(SEND_ERROR "unknown compiler \"${CMAKE_CXX_COMPILER_ID}\"")
    endif()

    set(${result_var_name} "${WARNING_OPTIONS}" PARENT_SCOPE)
endfunction()

function(set_werror_flag result_var_name)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        list(APPEND ${result_var_name} "-Werror")
    elseif(
        CMAKE_CXX_COMPILER_ID STREQUAL "Clang"
        OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"
    )
        list(APPEND ${result_var_name} "-Werror")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        list(APPEND ${result_var_name} "/WX")
    else()
        message(SEND_ERROR "unknown compiler \"${CMAKE_CXX_COMPILER_ID}\"")
    endif()

    set(${result_var_name} "${${result_var_name}}" PARENT_SCOPE)
endfunction()
