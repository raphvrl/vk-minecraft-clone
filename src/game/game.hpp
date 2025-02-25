#pragma once

#include "core/window/window.hpp"
#include "core/camera/camera.hpp"
#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/texture.hpp"
#include "world/world.hpp"
#include "world/sky.hpp"
#include "world/outline.hpp"

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

    bool m_running;
};

} // namespace game