//
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/process.hpp"

#include "catch2_extra.hpp"
#include "ureact/adaptor/collect.hpp"
#include "ureact/transaction.hpp"

// on every std::pair<N, value> pass value N times
TEST_CASE( "ureact::process" )
{
    ureact::context ctx;

    auto src = ureact::make_source<std::pair<unsigned, int>>( ctx );
    ureact::events<int> processed;
    int calls = 0;

    const auto repeater = [&calls]( ureact::event_range<std::pair<unsigned, int>> range,
                              ureact::event_emitter<int> out ) {
        for( const auto& [n, value] : range )
            for( unsigned i = 0; i < n; ++i )
                out << value;
        ++calls;
    };

    SECTION( "Functional syntax" )
    {
        processed = ureact::process<int>( src, repeater );
    }
    SECTION( "Piped syntax" )
    {
        processed = src | ureact::process<int>( repeater );
    }

    const auto result = ureact::collect<std::vector>( processed );

    src.emit( { 2u, -1 } );
    src.emit( { 0u, 666 } );

    {
        ureact::transaction _{ ctx };
        src << std::make_pair( 1u, 2 ) << std::make_pair( 3u, 7 );
    }

    const std::vector<int> expected = { -1, -1, 2, 7, 7, 7 };
    CHECK( result.get() == expected );

    CHECK( calls == 3 );
}

// on every value read additional values N and timestamp and pass pair<timestamp, value> N times
TEST_CASE( "ureact::process (synced)" )
{
    ureact::context ctx;

    using record_t = std::pair<std::string, int>;

    auto src = ureact::make_source<int>( ctx );
    auto n = ureact::make_var<unsigned>( ctx, {} );
    auto timestamp = ureact::make_var<std::string>( ctx, {} );
    ureact::events<record_t> processed;

    const auto repeater = []( ureact::event_range<int> range,
                              unsigned n,
                              const std::string& timestamp,
                              ureact::event_emitter<record_t> out ) {
        for( const auto& value : range )
            for( unsigned i = 0; i < n; ++i )
                out << record_t{ timestamp, value };
    };

    SECTION( "Functional syntax" )
    {
        processed = ureact::process<record_t>( src, with( n, timestamp ), repeater );
    }
    SECTION( "Piped syntax" )
    {
        processed = src | ureact::process<record_t>( with( n, timestamp ), repeater );
    }

    const auto result = ureact::collect<std::vector>( processed );

    n <<= 2;
    timestamp <<= "1 Jan 2020";
    src.emit( -1 );

    n <<= 0;
    src.emit( 666 );

    {
        ureact::transaction _{ ctx };
        timestamp <<= "31 Feb 2021";
        n <<= 1;

        src( 2 );
        src( 7 );
    }

    const std::vector<record_t> expected = {
        { "1 Jan 2020", -1 },
        { "1 Jan 2020", -1 },
        { "31 Feb 2021", 2 },
        { "31 Feb 2021", 7 },
    };
    CHECK( result.get() == expected );
}
