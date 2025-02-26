#pragma once

#include "core/types.hpp"

namespace ecs
{

class ECS;

class System
{

public:
    System(ECS *ecs) : m_ecs(ecs) {}
    virtual ~System() = default;

    virtual void tick(f32 dt) = 0;

protected:
    ECS *m_ecs;


};

} // namespace ecs