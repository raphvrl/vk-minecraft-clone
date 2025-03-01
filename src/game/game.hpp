#pragma once

#include "core/window/window.hpp"
#include "core/camera/camera.hpp"
#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/texture.hpp"
#include "world/world.hpp"
#include "world/sky.hpp"
#include "world/outline.hpp"
#include "world/clouds.hpp"
#include "graphics/overlay_renderer.hpp"
#include "graphics/text.hpp"

#include "ecs/ecs.hpp"
#include "ecs/components/physics/transform.hpp"
#include "ecs/components/physics/velocity.hpp"
#include "ecs/components/physics/collider.hpp"
#include "ecs/components/player/player.hpp"
#include "ecs/systems/physics/physics.hpp"
#include "ecs/systems/player/player.hpp"

namespace game
{

class Game
{

public:
    Game();

    void init();
    void destroy();
    void run();

private:
    void handleInput();
    void tick(f32 dt);
    void render();

    core::Window m_window;
    core::Camera m_camera;

    gfx::VulkanCtx m_ctx;

    wld::World m_world;
    wld::Sky m_sky;
    wld::Outline m_outline;
    wld::Clouds m_clouds;

    gfx::OverlayRenderer m_overlay;
    gfx::Text m_text;

    ecs::ECS m_ecs;

    sys::Player m_playerSystem;
    sys::Physics m_physicsSystem;

    bool m_running;

    f32 m_frameTime;
};

} // namespace game