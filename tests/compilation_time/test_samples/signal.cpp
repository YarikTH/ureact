//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <ureact/signal.hpp>

#ifndef INCLUDE_ONLY

#    include <utility>

template <size_t N>
struct Int
{
    int value;

    operator int()
    {
        return value;
    }
};

using namespace ureact::default_context;

class SignalUser
{
public:
#    if USE_MEMBER_SIGNAL
    UREACT_USE_MEMBER_SIGNALS( SignalUser );
    template <typename S>
    using signal_t = member_signal<S>;
#    else
    template <typename S>
    using signal_t = ureact::signal<S>;
#    endif

#    if USE_DIFFERENT_TYPES
    template <size_t N>
    using Int = Int<N>;
#    else
    template <size_t N>
    using Int = int;
#    endif

#    if COPY_COUNT >= 1
    signal_t<Int<1>> value1
#        if INITIALIZE_SIGNALS
        = make_const( Int<1>{} );
#        else
        ;
#        endif
#    endif
#    if COPY_COUNT >= 2
    signal_t<Int<2>> value2
#        if INITIALIZE_SIGNALS
        = make_const( Int<2>{} );
#        else
        ;
#        endif
#    endif
#    if COPY_COUNT >= 3
    signal_t<Int<3>> value3
#        if INITIALIZE_SIGNALS
        = make_const( Int<3>{} );
#        else
        ;
#        endif
#    endif
#    if COPY_COUNT >= 4
    signal_t<Int<4>> value4
#        if INITIALIZE_SIGNALS
        = make_const( Int<4>{} );
#        else
        ;
#        endif
#    endif
#    if COPY_COUNT >= 5
    signal_t<Int<5>> value5
#        if INITIALIZE_SIGNALS
        = make_const( Int<5>{} );
#        else
        ;
#        endif
#    endif
#    if COPY_COUNT >= 6
    signal_t<Int<6>> value6
#        if INITIALIZE_SIGNALS
        = make_const( Int<6>{} );
#        else
        ;
#        endif
#    endif
#    if COPY_COUNT >= 7
    signal_t<Int<7>> value7
#        if INITIALIZE_SIGNALS
        = make_const( Int<7>{} );
#        else
        ;
#        endif
#    endif
#    if COPY_COUNT >= 8
    signal_t<Int<8>> value8
#        if INITIALIZE_SIGNALS
        = make_const( Int<8>{} );
#        else
        ;
#        endif
#    endif
#    if COPY_COUNT >= 9
    signal_t<Int<9>> value9
#        if INITIALIZE_SIGNALS
        = make_const( Int<9>{} );
#        else
        ;
#        endif
#    endif
#    if COPY_COUNT >= 10
    signal_t<Int<10>> value10
#        if INITIALIZE_SIGNALS
        = make_const( Int<10>{} );
#        else
        ;
#        endif
#    endif
#    if COPY_COUNT >= 11
    signal_t<Int<11>> value11
#        if INITIALIZE_SIGNALS
        = make_const( Int<11>{} );
#        else
        ;
#        endif
#    endif
#    if COPY_COUNT >= 12
#        error "COPY_COUNT should be less than 12"
#    endif
};

#endif

int main()
{}
