#include "player.hpp"
#include "ecs/ecs.hpp"

namespace sys
{

Player::Player(
    ecs::ECS *ecs,
    core::Window &window,
    core::Camera &camera,
    wld::World &world
)
    : System(ecs), m_window(window), m_camera(camera), m_world(world)
{
}

void Player::tick(f32 dt)
{
    if (m_playerEntity == ENTITY_NULL) {
        auto entities = m_ecs->view<cmp::Player>();
        if (!entities.empty()) {
            m_playerEntity = *entities.begin();
        } else {
            return;
        }
    }

    auto *player = m_ecs->getComponent<cmp::Player>(m_playerEntity);
    auto *transform = m_ecs->getComponent<cmp::Transform>(m_playerEntity);
    auto *velocity = m_ecs->getComponent<cmp::Velocity>(m_playerEntity);

    velocity->position = glm::vec3(0.0f);

    if (!player || !transform || !velocity) {
        return;
    }

    if (m_window.isKeyPressed(core::Key::W)) {
        glm::vec3 front = m_camera.getFront();
        front.y = 0.0f;

        front = glm::normalize(front);
        
        velocity->position += front * player->moveSpeed;
    }

    if (m_window.isKeyPressed(core::Key::S)) {
        glm::vec3 front = m_camera.getFront();
        front.y = 0.0f;

        front = glm::normalize(front);
        
        velocity->position -= front * player->moveSpeed;
    }

    if (m_window.isKeyPressed(core::Key::A)) {
        glm::vec3 right = m_camera.getRight();
        right.y = 0.0f;

        right = glm::normalize(right);
        
        velocity->position -= right * player->moveSpeed;
    }

    if (m_window.isKeyPressed(core::Key::D)) {
        glm::vec3 right = m_camera.getRight();
        right.y = 0.0f;

        right = glm::normalize(right);
        
        velocity->position += right * player->moveSpeed;
    }

    if (m_window.isKeyPressed(core::Key::SPACE)) {
        velocity->position.y += player->moveSpeed;
    }

    if (m_window.isKeyPressed(core::Key::LSHIFT)) {
        velocity->position.y -= player->moveSpeed;
    }

    if (
        m_window.isMouseButtonPressed(core::MouseButton::LEFT) &&
        player->breakCooldown <= 0.0f
    ) {
        wld::Ray ray;

        ray.origin = m_camera.getPos();
        ray.direction = m_camera.getFront();

        wld::RaycastResult result;

        if (m_world.raycast(ray, 4.0f, result)) {
            m_world.deleteBlock(result.pos);
        }

        player->breakCooldown = 0.2f;
    }

    if (
        m_window.isMouseButtonPressed(core::MouseButton::RIGHT) &&
        player->placeCooldown <= 0.0f
    ) {
        wld::Ray ray;

        ray.origin = m_camera.getPos();
        ray.direction = m_camera.getFront();

        wld::RaycastResult result;

        if (m_world.raycast(ray, 4.0f, result)) {
            m_world.placeBlock(
                result.normal,
                wld::BlockType::COBBLESTONE
            );
        }

        player->placeCooldown = 0.2f;
    }

    if (player->breakCooldown > 0.0f) {
        player->breakCooldown -= dt;
    }
    
    if (player->placeCooldown > 0.0f) {
        player->placeCooldown -= dt;
    }

    glm::vec3 pos = transform->position;
    pos.y += player->eyeHeight;
    m_camera.setPos(pos);

    glm::vec2 mouse = m_window.getMouseRel();
    m_camera.rotate(
        mouse.x * player->sensitivity,
        mouse.y * player->sensitivity
    );
}

} // namespace sys