#include "physics.hpp"
#include "ecs/ecs.hpp"

namespace sys
{

Physics::Physics(ecs::ECS *ecs)
    : System(ecs)
{
}

void Physics::tick(f32 dt)
{
    auto entities = m_ecs->view<cmp::Transform, cmp::Velocity>();

    for (auto entity : entities) {
        auto *velocity = m_ecs->getComponent<cmp::Velocity>(entity);

        velocity->position *= dt;
    }
}

} // namespace sys