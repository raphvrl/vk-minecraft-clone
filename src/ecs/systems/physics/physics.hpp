#pragma once

#include "ecs/systems/system.hpp"
#include "world/world.hpp"

#include "ecs/components/physics/transform.hpp"
#include "ecs/components/physics/velocity.hpp"
#include "ecs/components/physics/collider.hpp"
#include "ecs/components/player/player.hpp"

namespace sys
{

class Physics : public ecs::System
{

public:
    Physics(ecs::ECS *ecs, wld::World &world);

    void tick(f32 dt) override;

private:
    wld::World &m_world;

    static constexpr f32 GRAVITY = -19.6f;

    void resolveCollisions(
        cmp::Transform *transform,
        cmp::Velocity *velocity,
        cmp::Collider *collider,
        f32
    );

};

} // namespace sys
