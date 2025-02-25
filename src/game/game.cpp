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
    f32 dt = m_window.getDeltaTime();
    f32 speed = 10.0f * dt;

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

    static usize left_couldown = 0;
    if (left_couldown > 0) {
        left_couldown -= dt;
    }

    if (
        m_window.isMouseButtonPressed(core::MouseButton::LEFT) &&
        left_couldown == 0
    ) {
        wld::Ray ray;
        ray.origin = m_camera.getPos();
        ray.direction = m_camera.getFront();

        wld::RaycastResult result;
        if (m_world.raycast(ray, 5.0f, result)) {
            m_world.deleteBlock(result.pos);
            left_couldown = 300;
        }
    }

    static usize right_couldown = 0;
    if (right_couldown > 0) {
        right_couldown -= dt;
    }

    if (
        m_window.isMouseButtonPressed(core::MouseButton::RIGHT) &&
        right_couldown == 0
    ) {
        wld::Ray ray;
        ray.origin = m_camera.getPos();
        ray.direction = m_camera.getFront();

        wld::RaycastResult result;
        if (m_world.raycast(ray, 5.0f, result)) {
            m_world.placeBlock(result.normal, wld::BlockType::STONE);
            right_couldown = 300;
        }
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
    m_outline.render(m_camera);

    m_ctx.endFrame();
}

} // namespace game