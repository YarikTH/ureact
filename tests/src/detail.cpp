//
//         Copyright (C) 2020-2023 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
#include <optional>
#include <set>
#include <tuple>

#include "catch2_extra.hpp"
#include "ureact/detail/node.hpp"
#include "ureact/detail/slot_map.hpp"

// non-copyable and non-movable
static_assert( !std::is_copy_constructible_v<ureact::detail::node> );
static_assert( !std::is_copy_assignable_v<ureact::detail::node> );
static_assert( !std::is_move_constructible_v<ureact::detail::node> );
static_assert( !std::is_move_assignable_v<ureact::detail::node> );

namespace
{

class detector
{
public:
    detector( std::set<int>* aliveIndices, int value )
        : alive_indices( aliveIndices )
        , value( value )
    {
        aliveIndices->insert( value );
    }

    ~detector()
    {
        if( has_value )
        {
            alive_indices->erase( value );
        }
    }

    UREACT_MAKE_NONCOPYABLE( detector );

    detector( detector&& other )
        : alive_indices( other.alive_indices )
        , value( other.value )
    {
        other.has_value = false;
    }

    detector& operator=( detector&& other ) = delete;

    [[nodiscard]] int get_value() const
    {
        return value;
    }

    [[nodiscard]] int get_value_non_const()
    {
        return value;
    }

private:
    std::set<int>* alive_indices;
    int value;
    bool has_value = true;
};

} // namespace

TEST_CASE( "ureact::detail::slot_map" )
{
    std::set<int> alive_indices;

    SECTION( "Initial state" )
    {
        ureact::detail::slot_map<detector> slot_map;

        CHECK( slot_map.empty() );
        CHECK( slot_map.size() == 0 );
        CHECK( slot_map.capacity() == 0 );
    }

    SECTION( "Emplace and then clean up" )
    {
        ureact::detail::slot_map<detector> slot_map;

        const auto newValueSlotId = slot_map.emplace( &alive_indices, 1 );
        CHECK_FALSE( slot_map.empty() );
        CHECK( slot_map.size() == 1 );
        CHECK( slot_map.capacity() > 0 );

        CHECK( alive_indices == std::set<int>{ 1 } );

        const auto initialCapacity = slot_map.capacity();

        SECTION( "erase it" )
        {
            slot_map.erase( newValueSlotId );

            CHECK( slot_map.capacity() == initialCapacity );
        }

        SECTION( "clear" )
        {
            slot_map.clear();

            CHECK( slot_map.capacity() == initialCapacity );
        }

        SECTION( "reset" )
        {
            slot_map.reset();

            CHECK( slot_map.capacity() == 0 );
        }

        CHECK( slot_map.empty() );
        CHECK( slot_map.size() == 0 );
        CHECK( alive_indices == std::set<int>{} );
    }

    SECTION( "Access" )
    {
        ureact::detail::slot_map<detector> slot_map;

        const auto newValueSlotId = slot_map.emplace( &alive_indices, 42 );
        CHECK_FALSE( slot_map.empty() );
        CHECK( slot_map.size() == 1 );

        const auto get_value = []( const ureact::detail::slot_map<detector>& from, size_t slotId ) {
            return from[slotId].get_value();
        };

        const auto get_value_non_const
            = []( ureact::detail::slot_map<detector>& from, size_t slotId ) {
                  return from[slotId].get_value_non_const();
              };

        CHECK( get_value( slot_map, newValueSlotId ) == 42 );
        CHECK( get_value_non_const( slot_map, newValueSlotId ) == 42 );
    }

    SECTION( "Destructor clears all existing values" )
    {
        {
            ureact::detail::slot_map<detector> slot_map;
            const auto firstValueSlot = slot_map.emplace( &alive_indices, -1 );
            std::ignore = slot_map.emplace( &alive_indices, 1 );
            slot_map.erase( firstValueSlot ); // make a hole, so we can see if it is handled

            CHECK( alive_indices == std::set<int>{ 1 } );
        }

        CHECK( alive_indices == std::set<int>{} );
    }

    SECTION( "Move construction" )
    {
        ureact::detail::slot_map<detector> slot_map;
        const auto firstValueSlot = slot_map.emplace( &alive_indices, -1 );
        std::ignore = slot_map.emplace( &alive_indices, 1 );
        slot_map.erase( firstValueSlot ); // make a hole, so we can see if it is handled

        CHECK_FALSE( slot_map.empty() );
        CHECK( slot_map.size() == 1 );
        CHECK( slot_map.capacity() > 0 );
        CHECK( alive_indices == std::set<int>{ 1 } );
        const auto originalCapacity = slot_map.capacity();

        ureact::detail::slot_map<detector> slot_map2{ std::move( slot_map ) };

        // values are intact
        CHECK( alive_indices == std::set<int>{ 1 } );

        // new slot_map has borrowed 1 value and all capacity
        CHECK_FALSE( slot_map2.empty() );
        CHECK( slot_map2.size() == 1 );
        CHECK( slot_map2.capacity() == originalCapacity );

        // Old slot map should be in initial state
        CHECK( slot_map.empty() );
        CHECK( slot_map.size() == 0 );
        CHECK( slot_map.capacity() == 0 );
    }

    SECTION( "Move assignment" )
    {
        ureact::detail::slot_map<detector> slot_map;
        const auto firstValueSlot = slot_map.emplace( &alive_indices, -1 );
        std::ignore = slot_map.emplace( &alive_indices, 1 );
        slot_map.erase( firstValueSlot );

        CHECK_FALSE( slot_map.empty() );
        CHECK( slot_map.size() == 1 );
        CHECK( slot_map.capacity() > 0 );
        CHECK( alive_indices == std::set<int>{ 1 } );
        const auto originalCapacity = slot_map.capacity();

        SECTION( "from self" )
        {
            UREACT_CLANG_SUPPRESS_WARNING_WITH_PUSH( "-Wself-move" )
            slot_map = std::move( slot_map );
            UREACT_CLANG_SUPPRESS_WARNING_POP

            // slot_map should stay intact
            CHECK_FALSE( slot_map.empty() );
            CHECK( slot_map.size() == 1 );
            CHECK( slot_map.capacity() == originalCapacity );
            CHECK( alive_indices == std::set<int>{ 1 } );
        }

        SECTION( "from other" )
        {
            ureact::detail::slot_map<detector> slot_map2;
            std::ignore = slot_map2.emplace( &alive_indices, 10 );
            const auto secondValueSlot = slot_map2.emplace( &alive_indices, -10 );
            std::ignore = slot_map2.emplace( &alive_indices, 11 );
            slot_map2.erase( secondValueSlot );
            CHECK( alive_indices == std::set<int>{ 1, 10, 11 } );

            slot_map2 = std::move( slot_map );

            // values from slot_map2 are erased
            CHECK( alive_indices == std::set<int>{ 1 } );

            // new slot_map has borrowed 1 value and all capacity
            CHECK_FALSE( slot_map2.empty() );
            CHECK( slot_map2.size() == 1 );
            CHECK( slot_map2.capacity() == originalCapacity );

            // Old slot map should be in initial state
            CHECK( slot_map.empty() );
            CHECK( slot_map.size() == 0 );
            CHECK( slot_map.capacity() == 0 );
        }
    }

    SECTION( "Growth" )
    {
        ureact::detail::slot_map<detector> slot_map;

        int i = 0;
        std::ignore = slot_map.emplace( &alive_indices, i++ );
        CHECK( slot_map.capacity() > 0 );

        // Fill up values to match capacity
        const auto initialCapacity = slot_map.capacity();
        while( slot_map.size() < initialCapacity )
        {
            std::ignore = slot_map.emplace( &alive_indices, i++ );
        }

        CHECK( alive_indices.size() == slot_map.size() );
        CHECK( slot_map.capacity() == initialCapacity );

        // Emplace one more
        std::ignore = slot_map.emplace( &alive_indices, i++ );
        CHECK( slot_map.capacity() > initialCapacity );
        CHECK( alive_indices.size() == slot_map.size() );
    }
}
