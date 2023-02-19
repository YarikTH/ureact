//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_FLATTEN_HPP
#define UREACT_ADAPTOR_FLATTEN_HPP

#include <ureact/detail/adaptor.hpp>
#include <ureact/detail/base.hpp>
#include <ureact/equal_to.hpp>
#include <ureact/type_traits.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename S>
class signal_node;

template <typename E>
class event_stream_node;

template <typename OuterS, typename InnerS>
class signal_flatten_node final : public signal_node<InnerS>
{
public:
    signal_flatten_node( context& context,
        std::shared_ptr<signal_node<OuterS>> outer,
        std::shared_ptr<signal_node<InnerS>> inner )
        : signal_flatten_node::signal_node( context, inner->value_ref() )
        , m_outer( std::move( outer ) )
        , m_inner( std::move( inner ) )
    {
        this->attach_to( m_outer->get_node_id() );
        this->attach_to( m_inner->get_node_id() );
    }

    ~signal_flatten_node() override
    {
        this->detach_from( m_inner->get_node_id() );
        this->detach_from( m_outer->get_node_id() );
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        {
            const auto& new_inner = m_outer->value_ref().get_node();
            if( !equal_to( new_inner, m_inner ) )
            {
                // Topology has been changed
                auto old_inner = m_inner;
                m_inner = new_inner;

                this->detach_from( old_inner->get_node_id() );
                this->attach_to( new_inner->get_node_id() );

                return update_result::shifted;
            }
        }

        return this->try_change_value( m_inner->value_ref() );
    }

private:
    std::shared_ptr<signal_node<OuterS>> m_outer;
    std::shared_ptr<signal_node<InnerS>> m_inner;
};

template <typename OuterS, typename InnerE>
class event_flatten_node final : public event_stream_node<InnerE>
{
public:
    event_flatten_node( context& context,
        std::shared_ptr<signal_node<OuterS>> outer,
        std::shared_ptr<event_stream_node<InnerE>> inner )
        : event_flatten_node::event_stream_node( context )
        , m_outer( std::move( outer ) )
        , m_inner( std::move( inner ) )
    {
        this->attach_to( m_outer->get_node_id() );
        this->attach_to( m_inner->get_node_id() );
    }

    ~event_flatten_node() override
    {
        this->detach_from( m_inner->get_node_id() );
        this->detach_from( m_outer->get_node_id() );
    }

    UREACT_WARN_UNUSED_RESULT update_result update() override
    {
        const auto& new_inner = m_outer->value_ref().get_node();
        if( !equal_to( new_inner, m_inner ) )
        {
            // Topology has been changed
            auto old_inner = m_inner;
            m_inner = new_inner;

            this->detach_from( old_inner->get_node_id() );
            this->attach_to( new_inner->get_node_id() );

            return update_result::shifted;
        }

        this->events().insert(
            this->events().end(), m_inner->events().begin(), m_inner->events().end() );

        return !this->events().empty() ? update_result::changed : update_result::unchanged;
    }

private:
    std::shared_ptr<signal_node<OuterS>> m_outer;
    std::shared_ptr<event_stream_node<InnerE>> m_inner;
};

struct FlattenClosure : AdaptorClosure
{
    /*!
	 * @brief Create a new event stream by flattening a signal
	 */
    template <typename InnerS>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()( const signal<InnerS>& outer ) const
    {
        context& context = outer.get_context();

        using value_t = typename InnerS::value_t;

        // clang-format off
	    using Node =
	        select_t<
	            condition<is_var_signal_v<InnerS>,   signal_flatten_node<InnerS, value_t>>,
	            condition<is_signal_v<InnerS>,       signal_flatten_node<InnerS, value_t>>,
	            condition<is_event_source_v<InnerS>, event_flatten_node<InnerS, value_t>>,
	            condition<is_event_v<InnerS>,        event_flatten_node<InnerS, value_t>>,
	            signature_mismatches>;
        // clang-format on

        static_assert( !std::is_same_v<Node, signature_mismatches>,
            "flatten: Passed signal does not match any of the supported signatures" );

        return InnerS{
            std::make_shared<Node>( context, outer.get_node(), outer.get().get_node() ) };
    }
};

} // namespace detail

inline constexpr detail::FlattenClosure flatten;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_FLATTEN_HPP
