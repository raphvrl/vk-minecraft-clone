#include "collision.hpp"
#include "ecs/ecs.hpp"

namespace sys
{

Collision::Collision(ecs::ECS *ecs, wld::World &world)
    : System(ecs), m_world(world)
{
}

void Collision::tick(f32 dt)
{
    auto entities = m_ecs->view<
        cmp::Transform,
        cmp::Velocity,
        cmp::Collider
    >();

    for (EntityID entity : entities) {
        auto *transform = m_ecs->getComponent<cmp::Transform>(entity);
        auto *velocity = m_ecs->getComponent<cmp::Velocity>(entity);
        auto *collider = m_ecs->getComponent<cmp::Collider>(entity);

        velocity->position *= dt;

        if (collider->isGhost) {
            continue;
        }

        resolveCollision(transform, velocity, collider);
    }
}

void Collision::resolveCollision(
    cmp::Transform *transform,
    cmp::Velocity *velocity,
    cmp::Collider *collider
)
{
    glm::vec3 testPos = transform->position;
    testPos.x += velocity->position.x;

    glm::vec3 min = testPos + collider->offset - collider->size * 0.5f;
    glm::vec3 max = testPos + collider->offset + collider->size * 0.5f;

    if (checkBlockCollision(min, max)) {
        velocity->position.x = 0.0f;
    } else {
        transform->position.x = testPos.x;
    }

    testPos = transform->position;
    testPos.y += velocity->position.y;

    min = testPos + collider->offset - collider->size * 0.5f;
    max = testPos + collider->offset + collider->size * 0.5f;

    if (checkBlockCollision(min, max)) {
        velocity->position.y = 0.0f;
    } else {
        transform->position.y = testPos.y;
    }

    testPos = transform->position;
    testPos.z += velocity->position.z;

    min = testPos + collider->offset - collider->size * 0.5f;
    max = testPos + collider->offset + collider->size * 0.5f;

    if (checkBlockCollision(min, max)) {
        velocity->position.z = 0.0f; 
    } else {
        transform->position.z = testPos.z;
    }
}

bool Collision::checkBlockCollision(const glm::vec3 &min, const glm::vec3 &max) const
{
    for (int x = static_cast<int>(std::floor(min.x)); x <= static_cast<int>(std::floor(max.x)); ++x) {
        for (int y = static_cast<int>(std::floor(min.y)); y <= static_cast<int>(std::floor(max.y)); ++y) {
            for (int z = static_cast<int>(std::floor(min.z)); z <= static_cast<int>(std::floor(max.z)); ++z) {
                if (m_world.getBlock(x, y, z) != wld::BlockType::AIR) {
                    return true;
                }
            }
        }
    }

    return false;
}

} // namespace sys