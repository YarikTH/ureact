#pragma once

#include <memory>
#include <vector>
#include <utility>

#include "i_reactive_node.hpp"

namespace ureact { namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////
/// i_observer
///////////////////////////////////////////////////////////////////////////////////////////////////
class i_observer
{
public:
    virtual ~i_observer() = default;

    virtual void unregister_self() = 0;

private:
    virtual void detach_observer() = 0;

    friend class observable;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
/// observable
///////////////////////////////////////////////////////////////////////////////////////////////////
class observable
{
public:
    observable() = default;
    
    observable(const observable&) = delete;
    observable& operator=(const observable&) = delete;
    observable(observable&&) noexcept = delete;
    observable& operator=(observable&&) noexcept = delete;
    
    ~observable()
    {
        for (const auto& p : observers_)
            if (p != nullptr)
                p->detach_observer();
    }

    void register_observer(std::unique_ptr<i_observer>&& obs_ptr)
    {
        observers_.push_back(std::move(obs_ptr));
    }

    void unregister_observer(i_observer* raw_obs_ptr)
    {
        for (auto it = observers_.begin(); it != observers_.end(); ++it)
        {
            if (it->get() == raw_obs_ptr)
            {
                it->get()->detach_observer();
                observers_.erase(it);
                break;
            }
        }
    }

private:
    std::vector<std::unique_ptr<i_observer>> observers_;
};

}}
