//
//         Copyright (C) 2014-2017 Sebastian Jeckel.
//         Copyright (C) 2020-2023 Krylov Yaroslav.
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef UREACT_ADAPTOR_MERGE_HPP
#define UREACT_ADAPTOR_MERGE_HPP

#include <ureact/core/adaptor.hpp>
#include <ureact/events.hpp>

UREACT_BEGIN_NAMESPACE

namespace detail
{

template <typename E, typename... Values>
class event_merge_node final : public event_stream_node<E>
{
public:
    explicit event_merge_node( const context& context, const events<Values>&... sources )
        : event_merge_node::event_stream_node( context )
        , m_sources( sources... )
    {
        this->attach_to( sources... );
    }

    ~event_merge_node() override
    {
        this->detach_from_all();
    }

    UREACT_WARN_UNUSED_RESULT core::update_result update() override
    {
        std::apply(
            [this](
                const events<Values>&... sources ) { ( this->copy_events_from( sources ), ... ); },
            m_sources );

        return !this->get_events().empty() ? core::update_result::changed
                                           : core::update_result::unchanged;
    }

private:
    template <typename V>
    void copy_events_from( const events<V>& src )
    {
        const auto& src_events = get_internals( src ).get_events();
        this->get_events().insert( this->get_events().end(), src_events.begin(), src_events.end() );
    }

    std::tuple<events<Values>...> m_sources;
};

template <typename EIn = void>
struct MergeAdaptor : core::adaptor
{
    /// TODO: rewrite to something more sane. Unfortunately conditional_t doesn't work
    template <typename... Sources>
    static auto result_type_detector()
    {
        if constexpr( std::is_same_v<EIn, void> )
        {
            return std::common_type_t<Sources...>{};
        }
        else
        {
            return EIn{};
        }
    }

    /*!
	 * @brief Emit all events in source1, ... sources
	 *
	 *  @warning Not to be confused with std::merge() or ranges::merge()
	 */
    template <typename Source, typename... Sources>
    UREACT_WARN_UNUSED_RESULT constexpr auto operator()(
        const events<Source>& source1, const events<Sources>&... sources ) const
    {
        using E = decltype( result_type_detector<Source, Sources...>() );
        static_assert( sizeof...( Sources ) >= 1, "merge: 2+ arguments are required" );

        const context& context = source1.get_context();
        return detail::create_wrapped_node<events<E>, event_merge_node<E, Source, Sources...>>(
            context, source1, sources... );
    }
};

} // namespace detail

inline constexpr detail::MergeAdaptor<> merge;

template <typename EIn>
inline constexpr detail::MergeAdaptor<EIn> merge_as;

UREACT_END_NAMESPACE

#endif // UREACT_ADAPTOR_MERGE_HPP
