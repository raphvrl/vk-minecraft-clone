#pragma once

#include "core/window/window.hpp"
#include "core/camera/camera.hpp"
#include "ecs/systems/system.hpp"
#include "ecs/components/player/player.hpp"
#include "ecs/components/physics/transform.hpp"
#include "ecs/components/physics/velocity.hpp"
#include "ecs/components/physics/collider.hpp"
#include "world/world.hpp"
#include "graphics/overlay_renderer.hpp"

namespace sys
{

class Player : public ecs::System
{
public:
    Player(
        ecs::ECS *ecs,
        core::Window &window,
        core::Camera &camera,
        wld::World &world,
        gfx::OverlayRenderer &overlay
    );

    void tick(f32 dt) override;
    void updateCamera();

private:
    bool findPlayerEntity();

    void handleMovement(f32 dt, cmp::Player *player, cmp::Transform *transform, 
                       cmp::Velocity *velocity, cmp::Collider *collider);
    void handleJumping(f32 dt, cmp::Player *player, cmp::Transform *transform, 
                      cmp::Velocity *velocity, cmp::Collider *collider);
    void handleEnvironment(f32 dt, cmp::Player *player, cmp::Transform *transform);

    void handleSounds(f32 dt, cmp::Player *player, cmp::Transform *transform, 
                     cmp::Velocity *velocity, cmp::Collider *collider);
    void handleFootstepSounds(f32 dt, cmp::Transform *transform);
    void handleLandingSounds(
        cmp::Transform *transform,
        cmp::Velocity *velocity,
        cmp::Collider *collider
    );

    void handleBlockInteraction(f32 dt, cmp::Player *player, cmp::Transform *transform);
    void handleBlockBreaking(cmp::Player *player, cmp::Transform *transform);
    void handleBlockPlacement(cmp::Player *player, cmp::Transform *transform);

    wld::BlockType getBlockUnderPlayer(cmp::Transform *transform);
    void updateCooldowns(f32 dt, cmp::Player *player);
    bool wouldCollide(const glm::ivec3 &block, const glm::vec3 &pos, const glm::vec3 &size);

    EntityID m_playerEntity = ENTITY_NULL;
    core::Window &m_window;
    core::Camera &m_camera;
    wld::World &m_world;
    gfx::OverlayRenderer &m_overlay;
    

    f32 m_jumpCooldown = 0.0f;
    f32 m_footstepTimer = 0.0f;
    f32 m_lastY = 0.0f;
    f32 m_lastGroundedY = 0.0f;
    bool m_wasGrounded = false;
    bool m_hasJumped = false;
    bool m_jumpSoundPending = false;
};

} // namespace sys