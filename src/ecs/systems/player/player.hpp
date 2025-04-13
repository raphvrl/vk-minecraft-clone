#pragma once

#include "ecs/systems/system.hpp"
#include "core/window/window.hpp"
#include "core/camera/camera.hpp"
#include "world/world.hpp"
#include "graphics/overlay_renderer.hpp"

#include "ecs/components/physics/transform.hpp"
#include "ecs/components/physics/velocity.hpp"
#include "ecs/components/player/player.hpp"
#include "ecs/components/physics/collider.hpp"

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
    core::Window &m_window;
    core::Camera &m_camera;
    wld::World &m_world;
    gfx::OverlayRenderer &m_overlay;

    EntityID m_playerEntity = ENTITY_NULL;

    f32 m_jumpCooldown = 0.0f;

    bool wouldCollide(
        const glm::ivec3 &block,
        const glm::vec3 &pos,
        const glm::vec3 &size
    );

    f32 m_footstepTimer = 0.0f;
    bool m_wasGrounded = false;
    f32 m_lastGroundedY = 0.0f;
    f32 m_fallingSince = 0.0f;
    f32 m_lastY = 0.0f;
};

} // namespace sys