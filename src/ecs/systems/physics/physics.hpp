#pragma once

#include "ecs/systems/system.hpp"

#include "ecs/components/physics/transform.hpp"
#include "ecs/components/physics/velocity.hpp"

namespace sys
{

class Physics : public ecs::System
{

public:
    Physics(ecs::ECS *ecs);

    void tick(f32 dt) override;

};

} // namespace sys
