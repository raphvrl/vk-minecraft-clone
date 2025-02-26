#pragma once

#include "ecs/systems/system.hpp"
#include "world/world.hpp"

#include "ecs/components/physics/transform.hpp"
#include "ecs/components/physics/velocity.hpp"
#include "ecs/components/physics/collider.hpp"

namespace sys
{

class Collision : public ecs::System
{

public:
    Collision(ecs::ECS *ecs, wld::World &world);

    void tick(f32 dt) override;

private:
    wld::World &m_world;

    void resolveCollision(
        cmp::Transform *transform,
        cmp::Velocity *velocity,
        cmp::Collider *collider
    );
};

} // namespace sys