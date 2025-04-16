#pragma once

#include "ecs/systems/system.hpp"
#include "world/world.hpp"

#include "ecs/components/physics/transform_component.hpp"
#include "ecs/components/physics/velocity_component.hpp"
#include "ecs/components/physics/collider_component.hpp"
#include "ecs/components/player/player_component.hpp"

namespace ecs
{

class PhysicsSystem : public ecs::System
{

public:
    PhysicsSystem(ecs::ECS *ecs, wld::World &world);

    void tick(f32 dt) override;

private:
    wld::World &m_world;

    void resolveCollisions(
        ecs::TransformComponent *transform,
        ecs::VelocityComponent *velocity,
        ecs::ColliderComponent *collider,
        f32
    );

};

} // namespace sys
