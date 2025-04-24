#pragma once

#include "core/window/window.hpp"
#include "core/camera/camera.hpp"

#include "graphics/device.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/gpu_data.hpp"
#include "graphics/texture_cache.hpp"
#include "graphics/display.hpp"
#include "graphics/renderer/overlay_renderer.hpp"

#include "gui/gui_manager.hpp"

#include "audio/sound_manager.hpp"

#include "world/core/world.hpp"
#include "world/environment/sky.hpp"
#include "world/environment/clouds.hpp"
#include "world/renderer/outline.hpp"

#include "admin/imgui_manager.hpp"
#include "admin/editor_manager.hpp"

#include "game/game_state.hpp"
#include "game/inventory.hpp"

#include "ecs/ecs.hpp"
#include "ecs/components/physics/transform_component.hpp"
#include "ecs/components/physics/velocity_component.hpp"
#include "ecs/components/physics/collider_component.hpp"
#include "ecs/components/player/player_component.hpp"
#include "ecs/systems/physics/physics_system.hpp"
#include "ecs/systems/player/player_system.hpp"

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
    static constexpr f64 MS_PER_TICK = 0.05;

    void handleInput();
    void update(f32 dt);
    void tick(f32 dt);
    void render();

    core::Window m_window;
    core::Camera m_camera;

    gfx::Device m_device;
    gfx::GPUData m_gpuData;
    gfx::TextureCache m_textureCache;
    gfx::Display m_display;

    gui::GUIManager m_guiManager;

    wld::World m_world;
    wld::Sky m_sky;
    wld::Outline m_outline;
    wld::Clouds m_clouds;

    gfx::OverlayRenderer m_overlay;

    ecs::ECS m_ecs;

    ecs::PlayerSystem m_playerSystem;
    ecs::PhysicsSystem m_physicsSystem;

    adm::ImGuiManager m_imguiManager;
    adm::EditorManager m_editorManager;

    bool m_running;

    f32 m_fps;

    GameState m_state = GameState::RUNNING;
    Inventory m_inventory;
};

} // namespace game