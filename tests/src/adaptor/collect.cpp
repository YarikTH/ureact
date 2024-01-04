//
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include "ureact/adaptor/collect.hpp"

#include <deque>
#include <list>
#include <map>
#include <set>

#include "catch2_extra.hpp"
#include "ureact/adaptor/enumerate.hpp"
#include "ureact/adaptor/transform.hpp"
#include "ureact/events.hpp"

// Collects received events into signal<ContT<E>>
TEST_CASE( "ureact::collect (sequential)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<int>( ctx );
    ureact::signal<std::vector<int>> collected_vec;
    ureact::signal<std::deque<int>> collected_deq;
    ureact::signal<std::list<int>> collected_lst;
    ureact::signal<std::set<int>> collected_set;

    SECTION( "Functional syntax" )
    {
        collected_vec = ureact::collect<std::vector>( src );
        collected_deq = ureact::collect<std::deque>( src );
        collected_lst = ureact::collect<std::list>( src );
        collected_set = ureact::collect<std::set>( src );
    }
    SECTION( "Piped syntax" )
    {
        collected_vec = src | ureact::collect<std::vector>;
        collected_deq = src | ureact::collect<std::deque>;
        collected_lst = src | ureact::collect<std::list>;
        collected_set = src | ureact::collect<std::set>;
    }

    for( int i : { 1, 2, 3 } )
        src << i;

    CHECK( collected_vec.get() == std::vector{ 1, 2, 3 } );
    CHECK( collected_deq.get() == std::deque{ 1, 2, 3 } );
    CHECK( collected_lst.get() == std::list{ 1, 2, 3 } );
    CHECK( collected_set.get() == std::set{ 1, 2, 3 } );
}

TEST_CASE( "ureact::collect (associative)" )
{
    ureact::context ctx;

    auto src = ureact::make_source<std::pair<std::string, int>>( ctx );
    auto src_tuple = ureact::transform(
        src, []( const auto& pair ) { return std::make_tuple( pair.first, pair.second ); } );

    ureact::signal<std::map<std::string, int>> collected_map;
    ureact::signal<std::map<std::string, int>> collected_map_2;
    ureact::signal<std::multimap<std::string, int>> collected_mmap;

    SECTION( "Functional syntax" )
    {
        collected_map = ureact::collect<std::map>( src );
        collected_map_2 = ureact::collect<std::map>( src_tuple );
        collected_mmap = ureact::collect<std::multimap>( src_tuple );
    }
    SECTION( "Piped syntax" )
    {
        collected_map = src | ureact::collect<std::map>;
        collected_map_2 = src_tuple | ureact::collect<std::map>;
        collected_mmap = src_tuple | ureact::collect<std::multimap>;
    }

    src << std::pair( "A", 1 );
    src << std::pair( "B", 10 );
    src << std::pair( "C", 3 );
    src << std::pair( "B", 2 );

    using namespace std::string_literals;
    const std::map<std::string, int> expected_map = {
        std::pair( "A"s, 1 ),
        std::pair( "B"s, 2 ),
        std::pair( "C"s, 3 ),
    };
    const std::multimap<std::string, int> expected_mmap = {
        std::pair( "A"s, 1 ),
        std::pair( "B"s, 10 ),
        std::pair( "B"s, 2 ),
        std::pair( "C"s, 3 ),
    };

    CHECK( collected_map.get() == expected_map );
    CHECK( collected_map_2.get() == expected_map );
    CHECK( collected_mmap.get() == expected_mmap );
}

TEST_CASE( "ureact::collect (with enumerate)" )
{
    ureact::context ctx;
    auto src = ureact::make_source<double>( ctx );
    auto enumerated = ureact::enumerate( src );

    ureact::signal collected_vec = enumerated | ureact::collect<std::vector>;
    ureact::signal collected_map = enumerated | ureact::collect<std::map>;

    for( int i : { 10, 20, 30 } )
        src << i;

    const std::vector<std::tuple<size_t, double>> expected_vec = { {
        { 0, 10.0 },
        { 1, 20.0 },
        { 2, 30.0 },
    } };
    const std::map<size_t, double> expected_map = {
        { 0, 10.0 },
        { 1, 20.0 },
        { 2, 30.0 },
    };

    CHECK( collected_vec.get() == expected_vec );
    CHECK( collected_map.get() == expected_map );
}
