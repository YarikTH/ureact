#pragma once

#include <memory>
#include <utility>
#include <cassert>

#include "ureact/detail/util.hpp"
#include "ureact/detail/i_reactive_node.hpp"
#include "ureact/detail/observer_base.hpp"
#include "ureact/detail/graph/observable_node.hpp"
#include "ureact/detail/graph/signal_observer_node.hpp"

namespace ureact {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename S>
class signal;

template <typename ... values_t>
class signal_pack;

using ::ureact::detail::observer_action;

/// An instance of this class provides a unique handle to an observer which can
/// be used to detach it explicitly. It also holds a strong reference to
/// the observed subject, so while it exists the subject and therefore
/// the observer will not be destroyed.
///
/// If the handle is destroyed without calling detach(), the lifetime of
/// the observer is tied to the subject.
class observer
{
private:
    using subject_ptr_t   = std::shared_ptr<::ureact::detail::observable_node>;
    using node_t         = ::ureact::detail::observer_node;

public:
    /// Default constructor
    observer() = default;

    /// Move constructor
    observer(observer&& other) noexcept :
        m_node_ptr( other.m_node_ptr ),
        m_subject_ptr( std::move( other.m_subject_ptr ) )
    {
        other.m_node_ptr = nullptr;
        other.m_subject_ptr.reset();
    }

    /// Node constructor
    observer(node_t* node_ptr, subject_ptr_t subject_ptr) :
        m_node_ptr( node_ptr ),
        m_subject_ptr( std::move( subject_ptr ) )
    {}

    /// Move assignment
    observer& operator=(observer&& other) noexcept
    {
        m_node_ptr = other.m_node_ptr;
        m_subject_ptr = std::move( other.m_subject_ptr );

        other.m_node_ptr = nullptr;
        other.m_subject_ptr.reset();

        return *this;
    }

    /// Deleted copy constructor and assignment
    observer(const observer&) = delete;
    observer& operator=(const observer&) = delete;
    
    ~observer() = default;
    
    /// Manually detaches the linked observer node from its subject
    void detach()
    {
        assert(is_valid());
        m_subject_ptr->unregister_observer( m_node_ptr );
    }

    /// Tests if this instance is linked to a node
    bool is_valid() const
    {
        return m_node_ptr != nullptr;
    }

private:
    /// Owned by subject
    node_t*          m_node_ptr = nullptr;

    /// While the observer handle exists, the subject is not destroyed
    subject_ptr_t     m_subject_ptr = nullptr;
};

/// Takes ownership of an observer and automatically detaches it on scope exit.
class scoped_observer
{
public:
    /// Move constructor
    scoped_observer(scoped_observer&& other)  noexcept :
        m_obs( std::move( other.m_obs ) )
    {}

    /// Constructs instance from observer
    scoped_observer(observer&& obs) :
        m_obs( std::move( obs ) )
    {}

    // Move assignment
    scoped_observer& operator=(scoped_observer&& other) noexcept
    {
        m_obs = std::move( other.m_obs );
        return *this;
    }

    /// Deleted default ctor, copy ctor and assignment
    scoped_observer() = delete;
    scoped_observer(const scoped_observer&) = delete;
    scoped_observer& operator=(const scoped_observer&) = delete;

    /// Destructor
    ~scoped_observer()
    {
        m_obs.detach();
    }

    /// Tests if this instance is linked to a node
    bool is_valid() const
    {
        return m_obs.is_valid();
    }

private:
    observer     m_obs;
};

/// When the signal value S of subject changes, func(s) is called.
/// The signature of func should be equivalent to:
/// TRet func(const S&)
/// TRet can be either observer_action or void.
/// By returning observer_action::stop_and_detach, the observer function can request
/// its own detachment. Returning observer_action::next keeps the observer attached.
/// Using a void return type is the same as always returning observer_action::next.
template
<
    typename in_f,
    typename S
>
auto observe(const signal<S>& subject, in_f&& func)
    -> observer
{
    using ::ureact::detail::i_observer;
    using observer_node = ::ureact::detail::observer_node;
    using ::ureact::detail::signal_observer_node;
    using ::ureact::detail::add_default_return_value_wrapper;

    using F = typename std::decay<in_f>::type;
    using R = typename std::result_of<in_f(S)>::type;
    using wrapper_t = add_default_return_value_wrapper<F,observer_action,observer_action::next>;

    // If return value of passed function is void, add observer_action::next as
    // default return value.
    using node_t = typename std::conditional<
        std::is_same<void,R>::value,
        signal_observer_node<S,wrapper_t>,
        signal_observer_node<S,F>
            >::type;

    const auto& subject_ptr = get_node_ptr(subject);

    std::unique_ptr<observer_node> node_ptr( new node_t(subject.get_context(), subject_ptr, std::forward<in_f>(func)) );
    observer_node* raw_node_ptr = node_ptr.get();

    subject_ptr->register_observer(std::move(node_ptr));

    return observer( raw_node_ptr, subject_ptr );
}

}
