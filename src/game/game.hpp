#pragma once

#include "core/window/window.hpp"
#include "core/camera/camera.hpp"
#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/texture.hpp"
#include "world/world.hpp"
#include "world/sky.hpp"
#include "world/outline.hpp"

#include "ecs/ecs.hpp"
#include "ecs/components/physics/transform.hpp"
#include "ecs/components/physics/velocity.hpp"
#include "ecs/components/physics/collider.hpp"
#include "ecs/components/player/player.hpp"
#include "ecs/systems/physics/physics.hpp"
#include "ecs/systems/physics/collision.hpp"
#include "ecs/systems/player/player.hpp"

namespace game
{

class Game
{

public:
    void init();
    void destroy();
    void run();

private:
    void handleInput();
    void update(f32 dt);
    void render();

    core::Window m_window;
    core::Camera m_camera;

    gfx::VulkanCtx m_ctx;

    wld::World m_world;
    wld::Sky m_sky;
    wld::Outline m_outline;

    ecs::ECS m_ecs;

    bool m_running;
};

} // namespace game