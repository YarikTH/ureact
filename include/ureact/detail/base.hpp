//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_DETAIL_BASE_HPP
#define UREACT_DETAIL_BASE_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <ureact/detail/algorithm.hpp>
#include <ureact/detail/defines.hpp>
#include <ureact/detail/slot_map.hpp>

UREACT_BEGIN_NAMESPACE

class context;

class transaction;

namespace detail
{
class context_internals;
}

/// TODO: looks ugly. Replace context_internals with more proper feature
UREACT_WARN_UNUSED_RESULT detail::context_internals& _get_internals( context& ctx );

namespace detail
{

class node_id
{
public:
    using value_type = size_t;

    node_id() = default;

    explicit node_id( value_type id )
        : m_id( id )
    {}

    operator value_type() // NOLINT
    {
        return m_id;
    }

    bool operator==( node_id other ) const noexcept
    {
        return m_id == other.m_id;
    }

    bool operator!=( node_id other ) const noexcept
    {
        return m_id != other.m_id;
    }

private:
    value_type m_id = -1;
};

enum class update_result
{
    unchanged,
    changed,
    shifted
};

struct reactive_node_interface
{
    virtual ~reactive_node_interface() = default;

    UREACT_WARN_UNUSED_RESULT virtual update_result update() = 0;

    /// Called after change propagation on changed nodes
    virtual void finalize()
    {}
};

class observer_interface
{
public:
    virtual ~observer_interface() = default;

    virtual void unregister_self() = 0;

private:
    virtual void detach_observer() = 0;

    friend class observable;
};

class observable
{
public:
    observable() = default;

    UREACT_MAKE_NONCOPYABLE( observable );
    UREACT_MAKE_NONMOVABLE( observable );

    ~observable()
    {
        for( const auto& p : m_observers )
            if( p != nullptr )
                p->detach_observer();
    }

    void register_observer( std::unique_ptr<observer_interface>&& obs_ptr )
    {
        m_observers.push_back( std::move( obs_ptr ) );
    }

    void unregister_observer( observer_interface* raw_obs_ptr )
    {
        for( auto it = m_observers.begin(); it != m_observers.end(); ++it )
        {
            if( it->get() == raw_obs_ptr )
            {
                it->get()->detach_observer();
                m_observers.erase( it );
                break;
            }
        }
    }

private:
    std::vector<std::unique_ptr<observer_interface>> m_observers;
};

/// Utility class to defer self detach of observers
class deferred_observer_detacher
{
public:
    void queue_observer_for_detach( observer_interface& obs )
    {
        m_detached_observers.push_back( &obs );
    }

protected:
    void detach_queued_observers()
    {
        for( observer_interface* o : m_detached_observers )
        {
            o->unregister_self();
        }
        m_detached_observers.clear();
    }

    std::vector<observer_interface*> m_detached_observers;
};

#if !defined( NDEBUG )
/// Utility class to check if callbacks passed in lift(), process() etc
/// are used properly
class callback_sanitizer
{
public:
    /// Return if external callback is in progress
    UREACT_WARN_UNUSED_RESULT bool is_locked() const
    {
        return m_is_locked;
    }

    /// Marks begin of an external callback
    void begin_external_callback()
    {
        assert( !m_is_locked );
        m_is_locked = true;
    }

    /// Marks end of an external callback
    void end_external_callback()
    {
        assert( m_is_locked );
        m_is_locked = false;
    }

    /// Marks a place where an external callback is called
    struct guard
    {
        callback_sanitizer& self;

        explicit guard( callback_sanitizer& self )
            : self( self )
        {
            self.begin_external_callback();
        }

        ~guard()
        {
            self.end_external_callback();
        }

        UREACT_MAKE_NONCOPYABLE( guard );
        UREACT_MAKE_NONMOVABLE( guard );
    };

private:
    bool m_is_locked = false;
};

#    define UREACT_CALLBACK_GUARD( _SELF_ ) callback_sanitizer::guard _( _SELF_ )
#else
#    define UREACT_CALLBACK_GUARD( _SELF_ )                                                        \
        do                                                                                         \
        {                                                                                          \
        } while( false )
#endif

class react_graph
    : public deferred_observer_detacher
#if !defined( NDEBUG )
    , public callback_sanitizer
#endif
{
public:
    react_graph() = default;

    node_id register_node( reactive_node_interface* nodePtr );
    void unregister_node( node_id nodeId );

    void attach_node( node_id nodeId, node_id parentId );
    void detach_node( node_id nodeId, node_id parentId );

    void push_input( node_id nodeId )
    {
        m_changed_inputs.push_back( nodeId );

        if( m_transaction_level == 0 )
        {
            propagate();
        }
    }

private:
    friend class ureact::transaction;

    void propagate()
    {
        std::vector<reactive_node_interface*> changed_nodes;

        // Fill update queue with successors of changed inputs
        for( node_id nodeId : m_changed_inputs )
        {
            auto& node = m_node_data[nodeId];
            auto* nodePtr = node.node_ptr;

            const update_result result = nodePtr->update();

            if( result == update_result::changed )
            {
                changed_nodes.push_back( nodePtr );
                schedule_successors( node );
            }
        }
        m_changed_inputs.clear();

        // Propagate changes
        while( m_scheduled_nodes.fetch_next() )
        {
            for( node_id nodeId : m_scheduled_nodes.next_values() )
            {
                auto& node = m_node_data[nodeId];
                auto* nodePtr = node.node_ptr;

                // A predecessor of this node has shifted to a lower level?
                if( node.level < node.new_level )
                {
                    // Re-schedule this node
                    node.level = node.new_level;

                    recalculate_successor_levels( node );
                    m_scheduled_nodes.push( nodeId, node.level );
                    continue;
                }

                const update_result result = nodePtr->update();

                // Topology changed?
                if( result == update_result::shifted )
                {
                    // Re-schedule this node
                    recalculate_successor_levels( node );
                    m_scheduled_nodes.push( nodeId, node.level );
                    continue;
                }

                if( result == update_result::changed )
                {
                    changed_nodes.push_back( nodePtr );
                    schedule_successors( node );
                }

                node.queued = false;
            }
        }

        // Cleanup buffers in changed nodes etc
        for( reactive_node_interface* nodePtr : changed_nodes )
            nodePtr->finalize();
        changed_nodes.clear();

        detach_queued_observers();
    }

    struct node_data
    {
        UREACT_MAKE_NONCOPYABLE( node_data );
        UREACT_MAKE_MOVABLE( node_data );

        explicit node_data( reactive_node_interface* node_ptr )
            : node_ptr( node_ptr )
        {}

        int level = 0;
        int new_level = 0;
        bool queued = false;

        reactive_node_interface* node_ptr = nullptr;

        std::vector<node_id> successors;
    };

    class topological_queue
    {
    public:
        using value_type = node_id;

        topological_queue() = default;

        void push( const value_type& value, const int level )
        {
            m_queue_data.emplace_back( value, level );
        }

        bool fetch_next();

        UREACT_WARN_UNUSED_RESULT const std::vector<value_type>& next_values() const
        {
            return m_next_data;
        }

    private:
        using entry = std::pair<value_type, int>;

        std::vector<value_type> m_next_data;
        std::vector<entry> m_queue_data;
    };

    void recalculate_successor_levels( node_data& node );

    void schedule_successors( node_data& node );

    slot_map<node_data> m_node_data;

    topological_queue m_scheduled_nodes;

    int m_transaction_level = 0;

    std::vector<node_id> m_changed_inputs;
};

inline node_id react_graph::register_node( reactive_node_interface* nodePtr )
{
    return node_id{ m_node_data.insert( node_data{ nodePtr } ) };
}

inline void react_graph::unregister_node( node_id nodeId )
{
    m_node_data.erase( nodeId );
}

UREACT_WARN_UNUSED_RESULT inline bool react_graph::topological_queue::fetch_next()
{
    // Throw away previous values
    m_next_data.clear();

    // Find min level of nodes in queue data
    auto minimal_level = std::numeric_limits<int>::max();
    for( const auto& e : m_queue_data )
    {
        if( minimal_level > e.second )
        {
            minimal_level = e.second;
        }
    }

    // Swap entries with min level to the end
    const auto p = detail::partition( m_queue_data.begin(),
        m_queue_data.end(),
        [minimal_level]( const entry& e ) { return e.second != minimal_level; } );

    // Reserve once to avoid multiple re-allocations
    const auto to_reserve = static_cast<size_t>( std::distance( p, m_queue_data.end() ) );
    m_next_data.reserve( to_reserve );

    // Move min level values to next data
    for( auto it = p, ite = m_queue_data.end(); it != ite; ++it )
    {
        m_next_data.push_back( it->first );
    }

    // Truncate moved entries
    const auto to_resize = static_cast<size_t>( std::distance( m_queue_data.begin(), p ) );
    m_queue_data.resize( to_resize );

    return !m_next_data.empty();
}

inline void react_graph::attach_node( node_id nodeId, node_id parentId )
{
    auto& node = m_node_data[nodeId];
    auto& parent = m_node_data[parentId];

    parent.successors.push_back( nodeId );

    if( node.level <= parent.level )
    {
        node.level = parent.level + 1;
    }
}

inline void react_graph::detach_node( node_id nodeId, node_id parentId )
{
    auto& parent = m_node_data[parentId];
    auto& successors = parent.successors;

    successors.erase( detail::find( successors.begin(), successors.end(), nodeId ) );
}

inline void react_graph::schedule_successors( node_data& node )
{
    // add children to queue
    for( node_id successorId : node.successors )
    {
        auto& successor = m_node_data[successorId];

        if( !successor.queued )
        {
            successor.queued = true;
            m_scheduled_nodes.push( successorId, successor.level );
        }
    }
}

inline void react_graph::recalculate_successor_levels( node_data& node )
{
    for( node_id successorId : node.successors )
    {
        auto& successor = m_node_data[successorId];

        if( successor.new_level <= node.level )
        {
            successor.new_level = node.level + 1;
        }
    }
}

class context_internals
{
public:
    context_internals() = default;

    UREACT_WARN_UNUSED_RESULT react_graph& get_graph()
    {
        return *m_graph;
    }

    UREACT_WARN_UNUSED_RESULT const react_graph& get_graph() const
    {
        return *m_graph;
    }

private:
    // context_internals and context should be non-movable because
    // node_base contains reference to context, and it will break if context lose its graph
    UREACT_MAKE_NONMOVABLE( context_internals );

    std::unique_ptr<react_graph> m_graph = std::make_unique<react_graph>();
};

class node_base : public reactive_node_interface
{
public:
    explicit node_base( context& context )
        : m_context( context )
    {
        assert( !get_graph().is_locked() && "Can't create node from callback" );
        m_id = get_graph().register_node( this );
    }

    ~node_base() override
    {
        get_graph().unregister_node( m_id );
    }

    UREACT_WARN_UNUSED_RESULT node_id get_node_id() const
    {
        return m_id;
    }

    UREACT_WARN_UNUSED_RESULT context& get_context() const
    {
        return m_context;
    }

    UREACT_WARN_UNUSED_RESULT react_graph& get_graph()
    {
        return _get_internals( m_context ).get_graph();
    }

    UREACT_WARN_UNUSED_RESULT const react_graph& get_graph() const
    {
        return _get_internals( m_context ).get_graph();
    }

protected:
    void attach_to( node_id parentId )
    {
        get_graph().attach_node( m_id, parentId );
    }

    void detach_from( node_id parentId )
    {
        get_graph().detach_node( m_id, parentId );
    }

private:
    UREACT_MAKE_NONCOPYABLE( node_base );

    context& m_context;

    node_id m_id;
};

class observer_node
    : public node_base
    , public observer_interface
{
public:
    explicit observer_node( context& context )
        : node_base( context )
    {}
};

class observable_node
    : public node_base
    , public observable
{
public:
    explicit observable_node( context& context )
        : node_base( context )
    {}
};

template <typename Node>
class reactive_base
{
public:
    reactive_base() = default;

    explicit reactive_base( std::shared_ptr<Node>&& node )
        : m_node( std::move( node ) )
    {}

    UREACT_WARN_UNUSED_RESULT bool is_valid() const
    {
        return m_node != nullptr;
    }

    UREACT_WARN_UNUSED_RESULT bool equal_to( const reactive_base& other ) const
    {
        return this->m_node == other.m_node;
    }

    UREACT_WARN_UNUSED_RESULT context& get_context() const
    {
        return m_node->get_context();
    }

    UREACT_WARN_UNUSED_RESULT const std::shared_ptr<Node>& get_node() const
    {
        return m_node;
    }

protected:
    std::shared_ptr<Node> m_node;
};

} // namespace detail

UREACT_END_NAMESPACE

#endif // UREACT_DETAIL_BASE_HPP
