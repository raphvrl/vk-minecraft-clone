#include "game.hpp"

namespace game
{

Game::Game() :
    m_playerSystem(&m_ecs, m_window, m_camera, m_world, m_overlay, m_inventory),
    m_physicsSystem(&m_ecs, m_world)
{
}

void Game::init()
{
    m_window.init(1600, 900, "Minecraft Clone");
    
    m_device.init(m_window, "Minecraft Clone", {0, 1, 0});
    m_gpuData.init(m_device);
    m_textureCache.init(m_device);

    m_textureCache.loadTexture("terrain.png", "terrain");
    m_textureCache.loadTexture("font.png", "font");
    m_textureCache.loadTexture("water.png", "water");
    m_textureCache.loadTexture("gui/gui.png", "gui");
    m_textureCache.loadTexture("gui/icons.png", "icons");

    m_display.init(m_device);

    m_guiManager.init(m_device, m_textureCache);

    sfx::SoundManager::get().init();

    m_world.init(m_device, m_textureCache);
    m_sky.init(m_device);
    m_outline.init(m_device, m_world);
    m_clouds.init(m_device);

    m_overlay.init(m_device, m_textureCache);

    m_imguiManager.init(m_device, m_window);
    m_guiEditor.init(m_imguiManager, m_guiManager, m_textureCache);

    m_inventory.load();
    m_guiManager.setInventory(m_inventory);

    EntityID playerEntity = m_ecs.creatEntity();
    auto transform = m_ecs.addComponent<ecs::TransformComponent>(playerEntity);
    transform->position = glm::vec3(0.0f, 127.0f, 0.0f);

    m_ecs.addComponent<ecs::VelocityComponent>(playerEntity);
    m_ecs.addComponent<ecs::PlayerComponent>(playerEntity);

    auto *playerCollider = m_ecs.addComponent<ecs::ColliderComponent>(playerEntity);

    playerCollider->size = glm::vec3(0.6f, 1.8f, 0.6f);
    playerCollider->offset = glm::vec3(0.0f, 0.9f, 0.0f);
    playerCollider->groundOffset = 0.01f;
    playerCollider->isGhost = false;

    m_running = true;
}

void Game::destroy()
{
    m_device.waitIdle();

    m_inventory.save();

    m_imguiManager.destroy();

    m_overlay.destroy();

    m_clouds.destroy();
    m_outline.destroy();
    m_sky.destroy();
    m_world.destroy();

    sfx::SoundManager::get().destroy();

    m_guiManager.destroy();

    m_display.destroy();
    m_textureCache.destroy();
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

    if (m_window.isKeyJustPressed(GLFW_KEY_F7)) {
        m_guiEditor.toggleVisible();
    }

    if (m_window.isKeyJustPressed(GLFW_KEY_F11)) {
        m_window.toogleFullscreen();
    }

    if (m_state == GameState::RUNNING) {
        for (int i = GLFW_KEY_1; i <= GLFW_KEY_9; i++) {
            if (m_window.isKeyJustPressed(i)) {
                m_inventory.setSelectedSlot(i - GLFW_KEY_1);
            }
        }

        f32 scroll = m_window.getMouseScrollY();
        if (scroll != 0.0f) {
            int currentSlot = m_inventory.getSelectedSlot();
            int newSlot;

            if (scroll > 0.0f) {
                newSlot = (currentSlot - 1 + 9) % 9;
            } else {
                newSlot = (currentSlot + 1) % 9;
            }

            m_inventory.setSelectedSlot(newSlot);
            m_window.resetMouseScrollY();
        }
    }
}

void Game::update(f32 dt)
{
    m_camera.updateView();
    m_camera.updateProj(m_window.getAspect());

    VkExtent2D extent = m_device.getExtent();
    m_camera.updateOrtho(extent.width, extent.height);

    m_gpuData.updateCamera(m_camera);

    m_gpuData.updateTime(m_window.getCurrentTime(), dt);

    m_gpuData.update();

    glm::vec3 camPos = m_camera.getPos();
    glm::vec3 camFront = m_camera.getFront();
    glm::vec3 camUp = m_camera.getUp();

    sfx::SoundManager::get().setListenerPos(camPos, camFront, camUp);
    sfx::SoundManager::get().update();
    
    if (m_state != GameState::RUNNING) {
        m_display.setColor({0.6f, 0.6f, 0.6f, 1.0f});
        return;
    } else {
        m_display.setColor({1.0f, 1.0f, 1.0f, 1.0f});
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

    m_display.begin(cmd);

    m_sky.render(cmd);
    m_world.render(m_camera, cmd);
    m_outline.render(cmd, m_camera);
    m_clouds.render(cmd, m_camera);
    m_overlay.render(cmd);
    m_guiManager.renderGame(cmd);

    m_display.end(cmd);

    m_device.beginRenderClear(cmd);
    m_display.draw(cmd);
    m_device.endRender(cmd);

    m_device.beginRenderLoad(cmd);

    m_imguiManager.beginFrame();

    m_guiManager.renderMenu(cmd);
    m_guiEditor.render();

    m_imguiManager.endFrame(cmd);

    m_device.endRender(cmd);
    m_device.endFrame(cmd);
}

} // namespace game