#include "game.hpp"
#include "gui/gui.hpp"

namespace game
{

Game::Game() :
    m_playerSystem(&m_ecs, m_window, m_camera, m_world, m_overlay),
    m_physicsSystem(&m_ecs, m_world)
{
}

void Game::init()
{
    m_window.init(1600, 900, "Minecraft Clone");
    
    m_device.init(m_window, "Minecraft Clone", {0, 1, 0});
    m_gpuData.init(m_device);

    m_world.init(m_device);
    m_sky.init(m_device);
    m_outline.init(m_device, m_world);
    m_clouds.init(m_device);

    m_overlay.init(m_device);

    m_gui.init(m_device);
    m_gui.setQuitCallback([&] {
        m_running = false;
    });
    m_gui.setResumeCallback([&] {
        m_state = GameState::RUNNING;
    });
    m_gui.initGameElements();
    m_gui.initPauseElements();

    EntityID playerEntity = m_ecs.creatEntity();
    m_ecs.addComponent<cmp::Transform>(playerEntity)
        ->position.y = 80.0f;
    m_ecs.addComponent<cmp::Velocity>(playerEntity);
    m_ecs.addComponent<cmp::Player>(playerEntity);
    auto *playerCollider = m_ecs.addComponent<cmp::Collider>(playerEntity);

    playerCollider->size = glm::vec3(0.6f, 1.8f, 0.6f);
    playerCollider->offset = glm::vec3(0.0f, 0.9f, 0.0f);
    playerCollider->groundOffset = 0.01f;
    playerCollider->isGhost = false;

    m_running = true;
}

void Game::destroy()
{
    m_device.waitIdle();

    m_gui.destroy();

    m_overlay.destroy();

    m_clouds.destroy();
    m_outline.destroy();
    m_sky.destroy();
    m_world.destroy();

    m_gpuData.destroy();
    m_device.destroy();

    m_window.destroy();
}

void Game::run()
{
    f64 lastTime = m_window.getCurrentTime();
    f64 accumulator = 0.0;
    
    int frameCount = 0;
    f64 fpsTimer = 0.0;
    f32 fps = 0.0;

    m_ecs.storePositions();
    
    while (m_running) {
        if (!m_window.isOpen()) {
            m_running = false;
        }

        f64 currentTime = m_window.getCurrentTime();
        f64 frameTime = currentTime - lastTime;
        lastTime = currentTime;

        frameCount++;
        fpsTimer += frameTime;
        if (fpsTimer >= 1.0) {
            fps = static_cast<f32>(frameCount) / static_cast<f32>(fpsTimer);
            frameCount = 0;
            fpsTimer = 0.0;
        }

        m_fps = fps;

        if (frameTime > 0.25) {
            frameTime = 0.25;
        }

        if (m_state == GameState::RUNNING) {
            accumulator += frameTime;
        }

        m_window.update();
        if (m_window.isMinimized()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        handleInput();

        while (accumulator >= MS_PER_TICK) {
            m_ecs.storePositions();
            tick(0.05f);
            accumulator -= MS_PER_TICK;
        }

        f32 alpha = static_cast<f32>(accumulator / MS_PER_TICK);

        update(alpha);
        
        render();
    }
}

void Game::handleInput()
{
    if (m_window.isKeyJustPressed(GLFW_KEY_ESCAPE)) {
        m_state = (m_state == GameState::RUNNING) ?
            GameState::PAUSED : GameState::RUNNING;
    }

    if (m_state == GameState::RUNNING) {
        m_window.setCursorMode(GLFW_CURSOR_DISABLED);
    } else {
        m_window.setCursorMode(GLFW_CURSOR_NORMAL);
    }

    if (m_window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        m_gui.handleMouseClick();
    }
}

void Game::update(f32 dt)
{
    updateGui();

    m_camera.updateView();
    m_camera.updateProj(m_window.getAspect());

    VkExtent2D extent = m_device.getExtent();
    m_camera.updateOrtho(extent.width, extent.height);

    m_gpuData.updateCamera(m_camera);
    m_gpuData.update();
    
    if (m_state != GameState::RUNNING) {
        return;
    }

    m_ecs.interpolate(dt);

    m_playerSystem.updateCamera();
}

void Game::tick(f32 dt)
{
    if (m_state != GameState::RUNNING) {
        return;
    }

    m_world.update(m_camera.getPos(), dt);
    m_clouds.update(dt);

    m_playerSystem.tick(dt);
    m_physicsSystem.tick(dt);
}

void Game::render()
{
    auto cmd = m_device.beginFrame();
    if (!cmd) {
        return;
    }

    m_sky.render(cmd);
    m_world.render(m_camera, cmd);
    m_outline.render(cmd, m_camera);
    m_clouds.render(cmd, m_camera);

    m_overlay.render(cmd);

    m_gui.render(cmd);

    m_device.endFrame();
}

void Game::updateGui()
{
    gui::GameStat gameStat;
    gameStat.fps = static_cast<u32>(m_fps);
    gameStat.updatedChunks = m_world.getUpdatedChunks();
    gameStat.state = m_state;

    m_gui.updateStat(gameStat);
    m_gui.update(m_window.getMousePos());
}

} // namespace game