//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/fold.hpp"

#include <numeric>

#include "catch2_extra.hpp"
#include "ureact/adaptor/count.hpp"
#include "ureact/adaptor/drop.hpp"
#include "ureact/adaptor/monitor.hpp"
#include "ureact/transaction.hpp"

// calculate sum and product of range of integers using value based function
TEST_CASE( "ureact::fold (by value)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::signal<int> sum;
    ureact::signal<int> product;
    int plus_calls = 0;
    int multiplies_calls = 0;

    auto plus = [&]( int value, int accum ) {
        ++plus_calls;
        return accum + value;
    };
    auto batch_multiplies = [&]( ureact::event_range<int> values, int accum ) {
        ++multiplies_calls;
        return std::accumulate( values.begin(), values.end(), accum, std::multiplies<>() );
    };

    SECTION( "Functional syntax" )
    {
        sum = ureact::fold( src, 0, plus );
        product = ureact::fold( src, 1, batch_multiplies );
    }
    SECTION( "Piped syntax" )
    {
        sum = src | ureact::fold( 0, plus );
        product = src | ureact::fold( 1, batch_multiplies );
    }

    std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // do a single transaction to show the difference
    {
        ureact::transaction _{ ctx };
        std::copy( v.begin(), v.end(), src.begin() );
    }

    CHECK( sum.get() == 55 );
    CHECK( product.get() == 3628800 );
    CHECK( plus_calls == 10 );
    CHECK( multiplies_calls == 1 );
}

// calculate sum and product of range of integers using reference based function
TEST_CASE( "ureact::fold (by reference)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::signal<int> sum_byval;
    ureact::signal<int> sum_byref;
    ureact::signal<int> product_byref;
    int plus_calls = 0;
    int multiplies_calls = 0;

    auto plus_ref = [&]( int value, int& accum ) {
        ++plus_calls;
        accum += value;
    };
    auto batch_multiplies_ref = [&]( ureact::event_range<int> values, int& accum ) {
        ++multiplies_calls;
        accum = std::accumulate( values.begin(), values.end(), accum, std::multiplies<>() );
    };

    SECTION( "Functional syntax" )
    {
        sum_byval = ureact::fold( src, 0, std::plus<>() );
        sum_byref = ureact::fold( src, 0, plus_ref );
        product_byref = ureact::fold( src, 1, batch_multiplies_ref );
    }
    SECTION( "Piped syntax" )
    {
        sum_byval = src | ureact::fold( 0, std::plus<>() );
        sum_byref = src | ureact::fold( 0, plus_ref );
        product_byref = src | ureact::fold( 1, batch_multiplies_ref );
    }

    auto sum_byval_changes = ureact::count( ureact::monitor( sum_byval ) );
    auto sum_byref_changes = ureact::count( ureact::monitor( sum_byref ) );

    std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // do a single transaction to show the difference
    {
        ureact::transaction _{ ctx };
        std::copy( v.begin(), v.end(), src.begin() );
    }

    CHECK( sum_byval.get() == 55 );
    CHECK( sum_byref.get() == 55 );
    CHECK( product_byref.get() == 3628800 );

    // value actually changed once
    CHECK( sum_byval_changes.get() == 1 );
    CHECK( sum_byref_changes.get() == 1 );

    // batch processing at work
    CHECK( plus_calls == 10 );
    CHECK( multiplies_calls == 1 );

    src.emit( 0 );
    src.emit( 0 );
    src.emit( 0 );

    CHECK( sum_byval.get() == 55 );
    CHECK( sum_byref.get() == 55 );
    CHECK( product_byref.get() == 0 );

    // passing 0 don't change the sum, but referenced fold reports change anyway
    CHECK( sum_byval_changes.get() == 1 );
    CHECK( sum_byref_changes.get() == 4 );

    // plus 3 calls each
    CHECK( plus_calls == 13 );
    CHECK( multiplies_calls == 4 );
}

// calculate sum of range of integers multiplied by using value based function
TEST_CASE( "ureact::fold (by value, synced)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto mult = ureact::make_var( ctx, 1 );
    ureact::signal<int> sum;
    ureact::signal<int> batch_sum;
    int plus_calls = 0;
    int batch_plus_calls = 0;

    auto plus = [&]( int value, int accum, int mult ) {
        ++plus_calls;
        return accum + value * mult;
    };
    auto batch_plus = [&]( ureact::event_range<int> values, int accum, int mult ) {
        ++batch_plus_calls;
        return accum + std::accumulate( values.begin(), values.end(), 0, std::plus<>() ) * mult;
    };

    SECTION( "Functional syntax" )
    {
        sum = ureact::fold( src, 0, with( mult ), plus );
        batch_sum = ureact::fold( src, 0, with( mult ), batch_plus );
    }
    SECTION( "Piped syntax" )
    {
        sum = src | ureact::fold( 0, with( mult ), plus );
        batch_sum = src | ureact::fold( 0, with( mult ), batch_plus );
    }

    std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // do a single transaction to show the difference
    {
        ureact::transaction _{ ctx };
        std::copy( v.begin(), v.end(), src.begin() );
    }

    CHECK( sum.get() == 55 );
    CHECK( batch_sum.get() == 55 );
    CHECK( plus_calls == 10 );
    CHECK( batch_plus_calls == 1 );

    {
        ureact::transaction _{ ctx };
        mult <<= 0;
        std::copy( v.begin(), v.end(), src.begin() );
    }

    CHECK( sum.get() == 55 );
    CHECK( batch_sum.get() == 55 );
    CHECK( plus_calls == 20 );
    CHECK( batch_plus_calls == 2 );

    {
        ureact::transaction _{ ctx };
        mult <<= 3;
        src << 5;
    }

    CHECK( sum.get() == 70 );
    CHECK( batch_sum.get() == 70 );
    CHECK( plus_calls == 21 );
    CHECK( batch_plus_calls == 3 );
}

// calculate sum of range of integers multiplied by using reference based function
TEST_CASE( "ureact::fold (by reference, synced)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    auto mult = ureact::make_var( ctx, 1 );
    ureact::signal<int> sum_byval;
    ureact::signal<int> sum_byref;
    ureact::signal<int> batch_sum_byref;
    int plus_calls = 0;
    int batch_plus_calls = 0;

    auto plus_val = []( int value, int accum, int mult ) { return accum + value * mult; };
    auto plus_ref = [&]( int value, int& accum, int mult ) {
        ++plus_calls;
        accum += value * mult;
    };
    auto batch_plus_ref = [&]( ureact::event_range<int> values, int& accum, int mult ) {
        ++batch_plus_calls;
        accum += std::accumulate( values.begin(), values.end(), accum, std::plus<>() ) * mult;
    };

    SECTION( "Functional syntax" )
    {
        sum_byval = ureact::fold( src, 0, with( mult ), plus_val );
        sum_byref = ureact::fold( src, 0, with( mult ), plus_ref );
        batch_sum_byref = ureact::fold( src, 0, with( mult ), batch_plus_ref );
    }
    SECTION( "Piped syntax" )
    {
        sum_byval = src | ureact::fold( 0, with( mult ), plus_val );
        sum_byref = src | ureact::fold( 0, with( mult ), plus_ref );
        batch_sum_byref = src | ureact::fold( 0, with( mult ), batch_plus_ref );
    }

    auto sum_byval_changes = ureact::count( ureact::monitor( sum_byval ) );
    auto sum_byref_changes = ureact::count( ureact::monitor( sum_byref ) );

    std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    // do a single transaction to show the difference
    {
        ureact::transaction _{ ctx };
        std::copy( v.begin(), v.end(), src.begin() );
    }

    CHECK( sum_byval.get() == 55 );
    CHECK( sum_byref.get() == 55 );
    CHECK( batch_sum_byref.get() == 55 );

    // value actually changed once
    CHECK( sum_byval_changes.get() == 1 );
    CHECK( sum_byref_changes.get() == 1 );

    // batch processing at work
    CHECK( plus_calls == 10 );
    CHECK( batch_plus_calls == 1 );

    mult <<= 0;
    src.emit( 1 );
    src.emit( 2 );
    src.emit( 3 );

    CHECK( sum_byval.get() == 55 );
    CHECK( sum_byref.get() == 55 );
    CHECK( batch_sum_byref.get() == 55 );

    // passing value multiplied by 0 don't change the sum, but referenced fold reports change anyway
    CHECK( sum_byval_changes.get() == 1 );
    CHECK( sum_byref_changes.get() == 4 );

    // plus 3 calls each
    CHECK( plus_calls == 13 );
    CHECK( batch_plus_calls == 4 );
}

// fold integers to receive sum, product and dash concatenated string
// based on example https://en.cppreference.com/w/cpp/algorithm/accumulate
TEST_CASE( "ureact::fold vs std::accumulate" )
{
    std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    const auto dash_fold =               //
        []( std::string accum, int i ) { //
            return std::move( accum ) + '-' + std::to_string( i );
        };

    // ureact::fold has inverse order of arguments compared with std::accumulate() for good reasons
    const auto dash_fold_2 =                       //
        [&dash_fold]( int i, std::string accum ) { //
            return dash_fold( std::move( accum ), i );
        };

    int sum = 0;
    int product = 0;
    std::string dashed;

    SECTION( "std::accumulate for example" )
    {
        sum = std::accumulate( v.begin(), v.end(), 0, std::plus<>() );

        product = std::accumulate( v.begin(), v.end(), 1, std::multiplies<>() );

        dashed = std::accumulate( std::next( v.begin(), 1 ),
            v.end(),
            std::to_string( v[0] ), // start with first element
            dash_fold );
    }
    SECTION( "ureact::fold" )
    {
        ureact::context ctx;

        auto src = ureact::make_source<int>( ctx );
        ureact::signal<int> sum_s;
        ureact::signal<int> product_s;
        ureact::signal<std::string> dashed_s;

        SECTION( "Functional syntax" )
        {
            sum_s = ureact::fold( src, 0, std::plus<>() );
            product_s = ureact::fold( src, 1, std::multiplies<>() );
            dashed_s = ureact::fold( ureact::drop( src, 1 ), //
                std::to_string( v[0] ),
                dash_fold_2 );
        }
        SECTION( "Piped syntax" )
        {
            sum_s = src | ureact::fold( 0, std::plus<>() );
            product_s = src | ureact::fold( 1, std::multiplies<>() );
            dashed_s = ureact::drop( src, 1 ) | ureact::fold( std::to_string( v[0] ), dash_fold_2 );
        }

        std::copy( v.begin(), v.end(), src.begin() );

        sum = sum_s.get();
        product = product_s.get();
        dashed = dashed_s.get();
    }

    CHECK( sum == 55 );
    CHECK( product == 3628800 );
    CHECK( dashed == "1-2-3-4-5-6-7-8-9-10" );
}
