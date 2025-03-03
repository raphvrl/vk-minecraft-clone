#include "player.hpp"
#include "ecs/ecs.hpp"

namespace sys
{

Player::Player(
    ecs::ECS *ecs,
    core::Window &window,
    core::Camera &camera,
    wld::World &world,
    gfx::OverlayRenderer &overlay
) : 
    System(ecs), 
    m_window(window),
    m_camera(camera),
    m_world(world),
    m_overlay(overlay)
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
    auto *collider = m_ecs->getComponent<cmp::Collider>(m_playerEntity);

    velocity->position.x = 0.0f;
    velocity->position.z = 0.0f;

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

    if (m_jumpCooldown > 0.0f) {
        m_jumpCooldown -= dt;
    }

    wld::BlockType headBlock = m_world.getBlock(
        transform->position.x,
        transform->position.y + player->eyeHeight,
        transform->position.z
    );

    wld::BlockType bodyBlock = m_world.getBlock(
        transform->position.x,
        transform->position.y + player->eyeHeight * 0.5f,
        transform->position.z
    );

    player->isInWater = (
        headBlock == wld::BlockType::WATER ||
        bodyBlock == wld::BlockType::WATER
    );

    m_overlay.setWater(headBlock == wld::BlockType::WATER);

    bool spacePressed = m_window.isKeyPressed(core::Key::SPACE);
    if (spacePressed && m_jumpCooldown <= 0.0f) {
        if (collider->isGrounded) {
            velocity->position.y = player->jumpForce;
            collider->isGrounded = false;
            m_jumpCooldown = 0.5f;
        }
    }

    if (m_window.isKeyPressed(core::Key::LSHIFT) && player->isInWater) {
        velocity->position.y = -player->swimSpeed * 0.8f;
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
            if (wouldCollide(
                result.normal,
                transform->position,
                collider->size)
            ) {
                return;
            }

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
}

void Player::updateCamera()
{
    if (m_playerEntity == ENTITY_NULL) return;

    auto *player = m_ecs->getComponent<cmp::Player>(m_playerEntity);
    auto *transform = m_ecs->getComponent<cmp::Transform>(m_playerEntity);

    if (!player || !transform) return;

    glm::vec3 pos = transform->renderPosition;
    pos.y += player->eyeHeight;
    m_camera.setPos(pos);

    glm::vec2 mouse = m_window.getMouseRel();
    m_camera.rotate(
        mouse.x * player->sensitivity,
        mouse.y * player->sensitivity
    );
}

bool Player::wouldCollide(
    const glm::ivec3 &block,
    const glm::vec3 &pos,
    const glm::vec3 &size
)
{
    f32 blockMinX = static_cast<f32>(block.x);
    f32 blockMaxX = static_cast<f32>(block.x + 1);
    f32 blockMinY = static_cast<f32>(block.y);
    f32 blockMaxY = static_cast<f32>(block.y + 1);
    f32 blockMinZ = static_cast<f32>(block.z);
    f32 blockMaxZ = static_cast<f32>(block.z + 1);

    f32 playerHalfWidth = size.x * 0.5f;
    f32 playerHalfDepth = size.z * 0.5f;

    f32 playerMinX = pos.x - playerHalfWidth;
    f32 playerMaxX = pos.x + playerHalfWidth;
    f32 playerMinY = pos.y;
    f32 playerMaxY = pos.y + size.y;
    f32 playerMinZ = pos.z - playerHalfDepth;
    f32 playerMaxZ = pos.z + playerHalfDepth;

    bool collisionX = playerMinX < blockMaxX && playerMaxX > blockMinX;
    bool collisionY = playerMinY < blockMaxY && playerMaxY > blockMinY;
    bool collisionZ = playerMinZ < blockMaxZ && playerMaxZ > blockMinZ;

    return collisionX && collisionY && collisionZ;
}

} // namespace sys