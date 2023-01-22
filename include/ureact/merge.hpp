//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_MERGE_HPP
#define UREACT_MERGE_HPP

#include <ureact/detail/base.hpp>
#include <ureact/detail/reactive_op_base.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename E, typename... Deps>
class event_merge_op : public reactive_op_base<Deps...>
{
public:
    template <typename... Args>
    explicit event_merge_op( Args&&... args )
        : event_merge_op::reactive_op_base( dont_move(), std::forward<Args>( args )... )
    {}

    UREACT_MAKE_MOVABLE( event_merge_op );

    template <typename Collector>
    void collect( const turn_type& turn, const Collector& collector ) const
    {
        std::apply( collect_functor<Collector>( turn, collector ), this->m_deps );
    }

    template <typename Collector, typename Functor>
    void collect_rec( const Functor& functor ) const
    {
        std::apply( reinterpret_cast<const collect_functor<Collector>&>( functor ), this->m_deps );
    }

private:
    template <typename Collector>
    struct collect_functor
    {
        collect_functor( const turn_type& turn, const Collector& collector )
            : m_turn( turn )
            , m_collector( collector )
        {}

        void operator()( const Deps&... deps ) const
        {
            ( collect( deps ), ... );
        }

        template <typename T>
        void collect( const T& op ) const
        {
            op.template collect_rec<Collector>( *this );
        }

        template <typename T>
        void collect( const std::shared_ptr<T>& dep_ptr ) const
        {
            dep_ptr->set_current_turn( m_turn );

            for( const auto& v : dep_ptr->events() )
            {
                m_collector( v );
            }
        }

        const turn_type& m_turn;
        const Collector& m_collector;
    };
};

template <typename E>
class event_stream_node;

template <typename E, typename Op>
class event_op_node;

template <typename E>
using event_stream_node_ptr_t = std::shared_ptr<event_stream_node<E>>;

} // namespace detail

/*!
 * @brief Emit all events in source1, ... sources
 *
 *  @warning Not to be confused with std::merge() or ranges::merge()
 */
template <typename Source, typename... Sources, typename E = Source>
UREACT_WARN_UNUSED_RESULT auto merge(
    const events<Source>& source1, const events<Sources>&... sources ) -> events<E>
{
    static_assert( sizeof...( Sources ) >= 1, "merge: 2+ arguments are required" );

    using Op = detail::event_merge_op<E,
        detail::event_stream_node_ptr_t<Source>,
        detail::event_stream_node_ptr_t<Sources>...>;

    context& context = source1.get_context();
    return events<E>( std::make_shared<detail::event_op_node<E, Op>>(
        context, source1.get_node(), sources.get_node()... ) );
}

UREACT_END_NAMESPACE

#endif // UREACT_MERGE_HPP
