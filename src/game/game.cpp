#include "game.hpp"

namespace game
{

void Game::init()
{
    m_window.init(1980, 1080, "Minecraft Clone");
    m_window.setCursorVisible(false);
    
    m_ctx.init(m_window);

    m_world.init(m_ctx);
    m_sky.init(m_ctx);

    m_running = true;
}

void Game::destroy()
{
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
    f32 speed = 25.0f * m_window.getDeltaTime();

    if (m_window.isKeyPressed(core::Key::ESCAPE)) {
        m_running = false;
    }

    if (m_window.isKeyPressed(core::Key::W)) {
        m_camera.moveForward(speed);
    }

    if (m_window.isKeyPressed(core::Key::S)) {
        m_camera.moveBackward(speed);
    }

    if (m_window.isKeyPressed(core::Key::A)) {
        m_camera.moveLeft(speed);
    }

    if (m_window.isKeyPressed(core::Key::D)) {
        m_camera.moveRight(speed);
    }

    if (m_window.isKeyPressed(core::Key::SPACE)) {
        m_camera.moveUp(speed);
    }

    if (m_window.isKeyPressed(core::Key::LSHIFT)) {
        m_camera.moveDown(speed);
    }

    static usize cooldown = 0;
    if (cooldown > 0) {
        cooldown -= m_window.getDeltaTime();
    }

    if (m_window.isMouseButtonPressed(core::MouseButton::LEFT) && cooldown == 0) {

        wld::Ray ray;
        ray.origin = m_camera.getPos();
        ray.direction = glm::normalize(m_camera.getFront());

        glm::ivec3 hitBlock;
        if (m_world.raycast(ray, 10.0f, hitBlock)) {
            m_world.deleteBlock(hitBlock);
        }

        cooldown = 10;
    }

    auto mouseRel = m_window.getMouseRel();
    m_camera.rotate(mouseRel.x, mouseRel.y);
}

void Game::update(f32 dt)
{
    UNUSED(dt);

    m_camera.update();
    m_world.update(m_camera.getPos());
}

void Game::render()
{
    if (!m_ctx.beginFrame()) {
        return;
    }

    m_sky.render(m_camera);
    m_world.render(m_camera);

    m_ctx.endFrame();
}

} // namespace game