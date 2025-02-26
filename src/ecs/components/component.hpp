#pragma once

#include "core/types.hpp"

namespace ecs
{

struct Component
{
    virtual ~Component() = default;
};

} // namespace ecs