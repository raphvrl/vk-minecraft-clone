#pragma once

#include "core/window/window.hpp"
#include "core/camera/camera.hpp"
#include "ecs/systems/system.hpp"
#include "ecs/components/player/player_component.hpp"
#include "ecs/components/physics/transform_component.hpp"
#include "ecs/components/physics/velocity_component.hpp"
#include "ecs/components/physics/collider_component.hpp"
#include "world/core/world.hpp"
#include "game/inventory.hpp"

#include "graphics/renderer/overlay_renderer.hpp"

namespace ecs
{

class PlayerSystem : public ecs::System
{
public:
    PlayerSystem(
        ecs::ECS *ecs,
        core::Window &window,
        core::Camera &camera,
        wld::World &world,
        gfx::OverlayRenderer &overlay,
        game::Inventory &inventory
    );

    void tick(f32 dt) override;
    void updateCamera();

private:
    bool findPlayerEntity();

    void handleMovement(
        f32 dt,
        ecs::PlayerComponent *player,
        ecs::TransformComponent *transform, 
        ecs::VelocityComponent *velocity,
        ecs::ColliderComponent *collider
    );
    void handleJumping(
        f32 dt,
        ecs::PlayerComponent *player, 
        ecs::TransformComponent *transform, 
        ecs::VelocityComponent *velocity, 
        ecs::ColliderComponent *collider
    );
    void handleEnvironment(
        f32 dt, 
        ecs::PlayerComponent *player,
        ecs::TransformComponent *transform
    );

    void handleSounds(
        f32 dt,
        ecs::PlayerComponent *player,
        ecs::TransformComponent *transform, 
        ecs::VelocityComponent *velocity,
        ecs::ColliderComponent *collider
    );
    void handleFootstepSounds(f32 dt, ecs::TransformComponent *transform);
    void handleLandingSounds(
        ecs::TransformComponent *transform,
        ecs::VelocityComponent *velocity,
        ecs::ColliderComponent *collider
    );

    void handleBlockInteraction(
        f32 dt,
        ecs::PlayerComponent *player,
        ecs::TransformComponent *transform
    );
    void handleBlockBreaking(
        ecs::PlayerComponent *player,
        ecs::TransformComponent *transform
    );
    void handleBlockPlacement(
        ecs::PlayerComponent *player,
        ecs::TransformComponent *transform
    );

    wld::BlockType getBlockUnderPlayer(ecs::TransformComponent *transform);
    void updateCooldowns(f32 dt, ecs::PlayerComponent *player);
    bool wouldCollide(
        const glm::ivec3 &block,
        const glm::vec3 &pos,
        const glm::vec3 &size
    );

    EntityID m_playerEntity = ENTITY_NULL;
    core::Window &m_window;
    core::Camera &m_camera;
    wld::World &m_world;
    gfx::OverlayRenderer &m_overlay;
    game::Inventory &m_inventory;
    
    f32 m_jumpCooldown = 0.0f;
    f32 m_footstepTimer = 0.0f;
    f32 m_lastY = 0.0f;
    f32 m_lastGroundedY = 0.0f;
    bool m_wasGrounded = false;
    bool m_hasJumped = false;
    bool m_jumpSoundPending = false;
};

} // namespace sys