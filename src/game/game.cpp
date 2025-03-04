#include "game.hpp"

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
    m_window.setCursorVisible(false);
    
    m_ctx.init(m_window);

    m_world.init(m_ctx);
    m_sky.init(m_ctx);
    m_outline.init(m_ctx, m_world);
    m_clouds.init(m_ctx);

    m_overlay.init(m_ctx);

    m_gui.init(m_ctx);

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
    m_gui.destroy();

    m_overlay.destroy();

    m_clouds.destroy();
    m_outline.destroy();
    m_sky.destroy();
    m_world.destroy();
    m_ctx.destroy();
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

        accumulator += frameTime;

        m_window.update();
        handleInput();

        while (accumulator >= MS_PER_TICK) {
            m_ecs.storePositions();
            tick(0.05f);
            accumulator -= MS_PER_TICK;
        }

        f32 alpha = static_cast<f32>(accumulator / MS_PER_TICK);

        m_ecs.interpolate(alpha);

        m_playerSystem.updateCamera();

        m_camera.updateView();
        m_camera.updateProj(m_window.getAspect());

        updateGui();

        render();
    }
}

void Game::handleInput()
{
    if (m_window.isKeyPressed(core::Key::ESCAPE)) {
        m_running = false;
    }
}

void Game::tick(f32 dt)
{
    m_world.update(m_camera.getPos(), dt);
    m_clouds.update(dt);

    m_playerSystem.tick(dt);
    m_physicsSystem.tick(dt);
}

void Game::render()
{
    if (!m_ctx.beginFrame()) {
        return;
    }

    m_sky.render(m_camera);
    m_world.render(m_camera);
    m_outline.render(m_camera);
    m_clouds.render(m_camera);

    m_overlay.render();

    m_gui.render();

    m_ctx.endFrame();
}

void Game::updateGui()
{
    gui::GameStat gameStat;
    gameStat.fps = static_cast<u32>(m_fps);
    gameStat.updatedChunks = m_world.getUpdatedChunks();

    m_gui.updateStat(gameStat);
    m_gui.update();
}

} // namespace game