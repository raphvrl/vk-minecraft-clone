#include "physics.hpp"
#include "ecs/ecs.hpp"

namespace sys
{

Physics::Physics(ecs::ECS *ecs, wld::World &world)
    : System(ecs), m_world(world)
{
}

void Physics::tick(f32 dt)
{
    auto entities = m_ecs->view<cmp::Transform, cmp::Velocity>();

    for (auto entity : entities) {
        auto *transform = m_ecs->getComponent<cmp::Transform>(entity);
        auto *velocity = m_ecs->getComponent<cmp::Velocity>(entity);
        auto *collider = m_ecs->getComponent<cmp::Collider>(entity);
        auto *player = m_ecs->getComponent<cmp::Player>(entity);

        if (player && !player->isFlying) {
            velocity->acceleration.y = GRAVITY;
        } else if (!player) {
            velocity->acceleration.y = GRAVITY;
        } else {
            velocity->acceleration.y = 0.0f;
        }

        velocity->position += velocity->acceleration * dt;

        if (collider && !collider->isGhost) {
            resolveCollisions(transform, velocity, collider, dt);
        } else {
            transform->position += velocity->position * dt;
        }
    }
}

void Physics::resolveCollisions(
    cmp::Transform *transform,
    cmp::Velocity *velocity,
    cmp::Collider *collider,
    f32 dt
)
{
    glm::vec3 movement = velocity->position * dt;

    if (glm::length(movement) < 0.001f) {
        return;
    }

    glm::vec3 testPos = transform->position;
    testPos.x += movement.x;

    glm::vec3 min = testPos + collider->offset - collider->size * 0.5f;
    glm::vec3 max = testPos + collider->offset + collider->size * 0.5f;

    if (m_world.checkCollision(min, max)) {
        velocity->position.x = 0.0f;
        movement.x = 0.0f;
    } else {
        transform->position.x = testPos.x;
    }

    testPos = transform->position;
    testPos.y += movement.y;

    min = testPos + collider->offset - collider->size * 0.5f;
    max = testPos + collider->offset + collider->size * 0.5f;

    if (m_world.checkCollision(min, max)) {
        if (velocity->position.y < 0.0f) {
            f32 low = transform->position.y;
            f32 hight = testPos.y;
            f32 mid;
            
            for (int i = 0; i < 10; i++) {
                mid = (low + hight) * 0.5f;
                testPos.y = mid;

                min = testPos + collider->offset - collider->size * 0.5f;
                max = testPos + collider->offset + collider->size * 0.5f;

                if (m_world.checkCollision(min, max)) {
                    hight = mid;
                } else {
                    low = mid;
                }
            }

            transform->position.y = low;
            collider->isGrounded = true;
        } else {
            f32 low = testPos.y;
            f32 hight = transform->position.y;
            f32 mid;

            for (int i = 0; i < 10; i++) {
                mid = (low + hight) * 0.5f;
                testPos.y = mid;

                min = testPos + collider->offset - collider->size * 0.5f;
                max = testPos + collider->offset + collider->size * 0.5f;

                if (m_world.checkCollision(min, max)) {
                    low = mid;
                } else {
                    hight = mid;
                }
            }

            transform->position.y = hight;
        }

        velocity->position.y = 0.0f;
    } else {
        transform->position.y = testPos.y;
        if (transform->position.y < 0.0f) {
            collider->isGrounded = true;
        }
    }

    testPos = transform->position;
    testPos.z += movement.z;

    min = testPos + collider->offset - collider->size * 0.5f;
    max = testPos + collider->offset + collider->size * 0.5f;

    if (m_world.checkCollision(min, max)) {
        velocity->position.z = 0.0f;
        movement.z = 0.0f;
    } else {
        transform->position.z = testPos.z;
    }

    if (!collider->isGrounded) {
        glm::vec3 groundCheck = transform->position;
        groundCheck.y -= collider->groundOffset;

        min = groundCheck + collider->offset - collider->size * 0.5f;
        min.y = groundCheck.y - collider->groundOffset;

        max = groundCheck + collider->offset + collider->size * 0.5f;
        max.y = min.y + 0.1f;

        collider->isGrounded = m_world.checkCollision(min, max);
    }
}

} // namespace sys