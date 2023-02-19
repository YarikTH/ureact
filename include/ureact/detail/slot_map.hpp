//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_SLOT_MAP_HPP
#define UREACT_DETAIL_SLOT_MAP_HPP

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include <ureact/detail/algorithm.hpp>
#include <ureact/detail/defines.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

/// A simple slot map
/// insert returns the slot index, which stays valid until the element is erased
/// TODO: test it thoroughly
/// TODO: there is a lot of places where placement new and placement delete are performed
///       need to use std::construct_at and std::destroy_at or their backports instead
/// TODO: maybe std::launder should be used instead of just reinterpret_cast
template <typename T>
class slot_map
{
public:
    using value_type = T;
    using size_type = size_t;
    using reference = value_type&;
    using const_reference = const value_type&;

    /// Constructs the slot_map
    slot_map() = default;

    /// Destructs the slot_map
    ~slot_map()
    {
        reset();
    }

    UREACT_MAKE_NONCOPYABLE( slot_map );
    UREACT_MAKE_MOVABLE( slot_map );

    /// Returns a reference to the element at specified slot index. No bounds checking is performed.
    reference operator[]( size_type index )
    {
        assert( has_index( index ) );
        return reinterpret_cast<reference>( m_data[index] );
    }

    /// Returns a reference to the element at specified slot index. No bounds checking is performed.
    const_reference operator[]( size_type index ) const
    {
        assert( has_index( index ) );
        return reinterpret_cast<const_reference>( m_data[index] );
    }

    /// Insert new object, return its index
    UREACT_WARN_UNUSED_RESULT size_type insert( value_type value )
    {
        if( is_at_full_capacity() )
        {
            grow();
            return insert_at_back( std::move( value ) );
        }
        else if( has_free_indices() )
        {
            return insert_at_freed_slot( std::move( value ) );
        }
        else
        {
            return insert_at_back( std::move( value ) );
        }
    }

    /// Destroy object by given index
    void erase( const size_type index )
    {
        assert( has_index( index ) );

        // If we erased something other than the last element, save in free index list.
        if( index != ( total_size() - 1 ) )
        {
            m_free_indices[m_free_size++] = index;
        }

        reinterpret_cast<reference>( m_data[index] ).~value_type();
        --m_size;

        // If free indices appeared at the end of allocated range, remove them from list
        shake_free_indices();
    }

    /// Clear the data, leave capacity intact
    void clear()
    {
        const size_type size = total_size();
        size_type index = 0;

        // Skip over free indices.
        for( size_type j = 0; j < m_free_size; ++j )
        {
            size_type free_index = m_free_indices[j];

            for( ; index < size; ++index )
            {
                if( index == free_index )
                {
                    ++index;
                    break;
                }
                else
                {
                    reinterpret_cast<reference>( m_data[index] ).~value_type();
                }
            }
        }

        // Rest
        for( ; index < size; ++index )
            reinterpret_cast<reference>( m_data[index] ).~value_type();

        m_size = 0;
        m_free_size = 0;
    }

    /// Clear the data and return container to its initial state with 0 capacity
    void reset()
    {
        clear();

        m_data.reset();
        m_free_indices.reset();

        m_capacity = 0;
    }

private:
    static inline constexpr size_t initial_capacity = 8;
    static inline constexpr size_t grow_factor = 2;

    using storage_type =
        typename std::aligned_storage<sizeof( value_type ), alignof( value_type )>::type;

    UREACT_WARN_UNUSED_RESULT bool is_at_full_capacity() const
    {
        return m_capacity == m_size;
    }

    UREACT_WARN_UNUSED_RESULT bool has_free_indices() const
    {
        return m_free_size > 0;
    }

    UREACT_WARN_UNUSED_RESULT size_type calculate_next_capacity() const
    {
        return m_capacity == 0 ? initial_capacity : m_capacity * grow_factor;
    }

    UREACT_WARN_UNUSED_RESULT size_type total_size() const
    {
        return m_size + m_free_size;
    }

    UREACT_WARN_UNUSED_RESULT bool is_free_index( const size_type index ) const
    {
        const auto begin_ = &m_free_indices[0];
        const auto end_ = &m_free_indices[m_free_size];

        const auto it = detail::find( begin_, end_, index );
        return it != end_;
    }

    UREACT_WARN_UNUSED_RESULT bool has_index( const size_type index ) const
    {
        return index < total_size() && !is_free_index( index );
    }

    void shake_free_indices()
    {
        if( m_free_size == 0 )
        {
            return;
        }

        detail::sort( &m_free_indices[0], &m_free_indices[m_free_size] );

        while( m_free_size > 0 && m_free_indices[m_free_size - 1] == total_size() - 1 )
        {
            --m_free_size;
        }
    }

    void grow()
    {
        // Allocate new storage
        const size_type new_capacity = calculate_next_capacity();

        std::unique_ptr<storage_type[]> new_data{ new storage_type[new_capacity] };
        std::unique_ptr<size_type[]> new_free_indices{ new size_type[new_capacity] };

        // Move data to new storage
        for( size_type i = 0; i < m_capacity; ++i )
        {
            new( reinterpret_cast<value_type*>( &new_data[i] ) )
                value_type{ std::move( reinterpret_cast<reference>( m_data[i] ) ) };
            reinterpret_cast<reference>( m_data[i] ).~value_type();
        }

        // Free list is empty if we are at max capacity anyway

        // Use new storage
        m_data = std::move( new_data );
        m_free_indices = std::move( new_free_indices );
        m_capacity = new_capacity;
    }

    size_type insert_at_back( value_type&& value )
    {
        new( &m_data[m_size] ) value_type( std::move( value ) );
        return m_size++;
    }

    size_type insert_at_freed_slot( value_type&& value )
    {
        const size_type next_free_index = m_free_indices[--m_free_size];
        new( &m_data[next_free_index] ) value_type( std::move( value ) );
        ++m_size;

        return next_free_index;
    }

    std::unique_ptr<storage_type[]> m_data;
    std::unique_ptr<size_type[]> m_free_indices;

    size_type m_size = 0;
    size_type m_free_size = 0;
    size_type m_capacity = 0;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_SLOT_MAP_HPP
