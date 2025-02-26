#include "game.hpp"

namespace game
{

void Game::init()
{
    m_window.init(1280, 720, "Minecraft Clone");
    m_window.setCursorVisible(false);
    
    m_ctx.init(m_window);

    m_world.init(m_ctx);
    m_sky.init(m_ctx);
    m_outline.init(m_ctx, m_world);

    EntityID playerEntity = m_ecs.creatEntity();
    m_ecs.addComponent<cmp::Transform>(playerEntity, cmp::Transform())
        ->position.y = 80.0f;
    m_ecs.addComponent<cmp::Velocity>(playerEntity, cmp::Velocity());
    m_ecs.addComponent<cmp::Player>(playerEntity, cmp::Player());

    m_ecs.addSystem<sys::Physics>();
    m_ecs.addSystem<sys::Player>(
        m_window,
        m_camera,
        m_world
    );

    m_running = true;
}

void Game::destroy()
{
    m_outline.destroy();
    m_sky.destroy();
    m_world.destroy();
    m_ctx.destroy();
    m_window.destroy();
}

void Game::run()
{
    while (m_running) {
        m_window.update();

        f32 dt = m_window.getDeltaTime();

        handleInput();
        update(dt);
        render();
    }
}

void Game::handleInput()
{
    if (m_window.isKeyPressed(core::Key::ESCAPE)) {
        m_running = false;
    }
}

void Game::update(f32 dt)
{
    m_camera.updateView();
    m_world.update(m_camera.getPos());

    m_ecs.update(dt);
}

void Game::render()
{
    if (!m_ctx.beginFrame()) {
        return;
    }

    m_sky.render(m_camera);
    m_world.render(m_camera);
    m_outline.render(m_camera);

    m_ctx.endFrame();
}

} // namespace game