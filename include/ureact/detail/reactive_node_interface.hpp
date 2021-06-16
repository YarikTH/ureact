#pragma once

#include <vector>

namespace ureact
{
namespace detail
{

class reactive_node
{
public:
    int level{ 0 };
    int new_level{ 0 };
    bool queued{ false };

    std::vector<reactive_node*> successors;

    virtual ~reactive_node() = default;

    virtual void tick() = 0;
};

struct input_node_interface
{
    virtual ~input_node_interface() = default;

    virtual bool apply_input() = 0;
};

} // namespace detail
} // namespace ureact
