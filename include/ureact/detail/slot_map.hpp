//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2024 Yaroslav Krylov.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_SLOT_MAP_HPP
#define UREACT_DETAIL_SLOT_MAP_HPP

#include <cassert>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include <ureact/detail/algorithm.hpp>
#include <ureact/detail/defines.hpp>
#include <ureact/detail/memory.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

/// A simple slot map
/// insert returns the slot index, which stays valid until the element is erased
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

    /// Custom move constructor
    slot_map( slot_map&& other )
        : m_storage( std::move( other.m_storage ) )
        , m_free_indices( std::move( other.m_free_indices ) )
        , m_size( other.m_size )
        , m_capacity( other.m_capacity )
    {
        other.m_size = 0;
        other.m_capacity = 0;
    }

    /// Custom move assign
    slot_map& operator=( slot_map&& other )
    {
        if( &other != this )
        {
            reset();

            m_storage = std::move( other.m_storage );
            m_free_indices = std::move( other.m_free_indices );
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    UREACT_MAKE_NONCOPYABLE( slot_map );

    /// Returns a reference to the element at specified slot index
    reference operator[]( size_type index )
    {
        assert( has_index( index ) );
        return *at( index );
    }

    /// Returns a constant reference to the element at specified slot index
    const_reference operator[]( size_type index ) const
    {
        assert( has_index( index ) );
        return *at( index );
    }

    /// Emplace new object, return its index
    template <class... Args>
    UREACT_WARN_UNUSED_RESULT size_type emplace( Args&&... args )
    {
        const size_type new_index = [&]() {
            if( !m_free_indices.empty() )
            {
                return m_free_indices.pop();
            }
            if( m_size == m_capacity )
            {
                grow();
            }
            return m_size;
        }();

        construct_at( new_index, std::forward<Args>( args )... );
        ++m_size;
        return new_index;
    }

    /// Destroy object by given index
    void erase( const size_type index )
    {
        assert( has_index( index ) );

        // If we erased something other than the last element, save in free index list.
        if( index != ( total_size() - 1 ) )
        {
            m_free_indices.push( index );
        }

        destroy_at( index );
        --m_size;

        // If free indices appeared at the end of allocated range, remove them from list
        m_free_indices.shake( m_size );
    }

    /// Return if there is any element inside
    UREACT_WARN_UNUSED_RESULT bool empty() const
    {
        return m_size == 0;
    }

    /// Return how many slots are used
    UREACT_WARN_UNUSED_RESULT size_type size() const
    {
        return m_size;
    }

    /// Return how many slots are allocated
    UREACT_WARN_UNUSED_RESULT size_type capacity() const
    {
        return m_capacity;
    }

    /// Clear the data, leave capacity intact
    void clear()
    {
        const size_type size = total_size();
        size_type index = 0;

        // Skip over sorted free indices.
        for( size_type free_index : m_free_indices )
        {
            for( ; index < size; ++index )
            {
                if( index == free_index )
                {
                    ++index;
                    break;
                }
                else
                {
                    destroy_at( index );
                }
            }
        }

        // Rest
        for( ; index < size; ++index )
        {
            destroy_at( index );
        }

        m_size = 0;
        m_free_indices.clear();
    }

    /// Clear the data and return container to its initial state with 0 capacity
    void reset()
    {
        clear();

        m_storage.reset();
        m_free_indices.reset();

        m_capacity = 0;
    }

private:
    class free_indices
    {
    public:
        free_indices() = default;

        explicit free_indices( const size_type amount )
            : m_data{ std::make_unique<size_type[]>( amount ) }
        {}

        /// Custom move constructor
        free_indices( free_indices&& other )
            : m_data( std::move( other.m_data ) )
            , m_size( other.m_size )
        {
            other.m_size = 0;
        }

        /// Custom move assign
        free_indices& operator=( free_indices&& other )
        {
            if( &other != this )
            {
                reset();

                m_data = std::move( other.m_data );
                m_size = other.m_size;
                other.m_size = 0;
            }
            return *this;
        }

        void reset()
        {
            m_data.reset();
            m_size = 0u;
        }

        UREACT_WARN_UNUSED_RESULT size_type amount() const
        {
            return m_size;
        }

        UREACT_WARN_UNUSED_RESULT bool empty() const
        {
            return m_size == 0;
        }

        void clear()
        {
            m_size = 0;
        }

        void push( const size_type index )
        {
            m_data[m_size++] = index;
            detail::sort( begin_(), end_() );
        }

        UREACT_WARN_UNUSED_RESULT size_type pop()
        {
            // TODO: It should take lowest free index instead to increase probability of successful shake.
            //       To keep it O(1) sorting order of m_data should be reversed.
            //       Or it should become pipebuffer to make adding/removing of new values O(1) from the both sides.
            //       Sounds too complicated to worth it.
            return m_data[--m_size];
        }

        void shake( const size_type elements_amount )
        {
            const auto total_size = [&]() { return elements_amount + m_size; };

            while( m_size > 0 && back_() == total_size() - 1 )
            {
                --m_size;
            }
        }

        UREACT_WARN_UNUSED_RESULT bool have( const size_type index ) const
        {
            const auto it = detail::find( begin(), end(), index );
            return it != end();
        }

        UREACT_WARN_UNUSED_RESULT const size_type* begin() const
        {
            return &m_data[0];
        }

        UREACT_WARN_UNUSED_RESULT const size_type* end() const
        {
            return &m_data[m_size];
        }

    private:
        UREACT_WARN_UNUSED_RESULT const size_type& back_() const
        {
            return m_data[m_size - 1];
        }

        UREACT_WARN_UNUSED_RESULT size_type* begin_()
        {
            return &m_data[0];
        }

        UREACT_WARN_UNUSED_RESULT size_type* end_()
        {
            return &m_data[m_size];
        }

        std::unique_ptr<size_type[]> m_data;
        size_type m_size = 0u;
    };

    static inline constexpr size_t initial_capacity = 8;
    static inline constexpr size_t grow_factor = 2;

    union storage_type
    {
        value_type data;

        storage_type()
        {}

        ~storage_type()
        {}
    };

    UREACT_WARN_UNUSED_RESULT size_type calculate_next_capacity() const
    {
        return m_capacity == 0 ? initial_capacity : m_capacity * grow_factor;
    }

    UREACT_WARN_UNUSED_RESULT size_type total_size() const
    {
        return m_size + m_free_indices.amount();
    }

    UREACT_WARN_UNUSED_RESULT bool has_index( const size_type index ) const
    {
        return index < total_size() && !m_free_indices.have( index );
    }

    void grow()
    {
        assert( m_size == m_capacity );
        assert( m_free_indices.empty() );

        // Allocate new storage
        const size_type new_capacity = calculate_next_capacity();

        auto new_storage = std::make_unique<storage_type[]>( new_capacity );

        // Move data to new storage
        // TODO: maybe should be replaced with std::uninitialized_move_n?
        for( size_type i = 0; i < m_capacity; ++i )
        {
            value_type* dst = std::addressof( new_storage[i].data );
            value_type* src = std::addressof( m_storage[i].data );
            detail::construct_at( dst, std::move( *src ) );
            detail::destroy_at( src );
        }

        // Free list is empty if we are at max capacity anyway

        // Use new storage
        m_storage = std::move( new_storage );
        m_free_indices = free_indices{ new_capacity };
        m_capacity = new_capacity;
    }

    value_type* at( size_type index )
    {
        return std::addressof( m_storage[index].data );
    }

    const value_type* at( size_type index ) const
    {
        return std::addressof( m_storage[index].data );
    }

    template <class... Args>
    void construct_at( const size_type index, Args&&... args )
    {
        detail::construct_at( at( index ), std::forward<Args>( args )... );
    }

    void destroy_at( const size_type index )
    {
        detail::destroy_at( at( index ) );
    }

    std::unique_ptr<storage_type[]> m_storage;
    free_indices m_free_indices;

    size_type m_size = 0;
    size_type m_capacity = 0;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_SLOT_MAP_HPP
