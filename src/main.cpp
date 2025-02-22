#include "core/window/window.hpp"
#include "utils/drawer/box_drawer.hpp"
#include "utils/console/console.hpp"
#include "core/camera/camera.hpp"
#include "graphics/vulkan/vulkan_ctx.hpp"
#include "graphics/vulkan/pipeline.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <locale.h>
#endif

int main()
{
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #else
        setlocale(LC_ALL, ".UTF-8");
    #endif

    core::Window window;

    window.init(1280, 720, "Vulkan");

    gfx::VulkanCtx ctx;
    ctx.init(window);

    gfx::Pipeline pipeline = gfx::Pipeline::Builder(ctx)
        .setShader(gfx::ShaderType::VERTEX, "default.vert.spv")
        .setShader(gfx::ShaderType::FRAGMENT, "default.frag.spv")
        .addPushConstant(gfx::ShaderType::VERTEX, 0, sizeof(glm::mat4))
        .build();

    core::Camera cam;

    glm::mat4 model(1.0f);

    while (window.isOpen()) {
        window.update();

        f32 dt = window.getDeltaTime();
        
        f32 speed = 2.5f * dt;

        if (window.isKeyPressed(core::Key::W)) {
            cam.moveForward(speed);
        }

        if (window.isKeyPressed(core::Key::S)) {
            cam.moveBackward(speed);
        }

        if (window.isKeyPressed(core::Key::A)) {
            cam.moveLeft(speed);
        }

        if (window.isKeyPressed(core::Key::D)) {
            cam.moveRight(speed);
        }

        if (window.isKeyPressed(core::Key::SPACE)) {
            cam.moveUp(speed);
        }

        if (window.isKeyPressed(core::Key::LSHIFT)) {
            cam.moveDown(speed);
        }

        cam.update();

        if (!ctx.beginFrame()) {
            continue;
        }

        glm::mat4 proj = cam.getProj();
        glm::mat4 view = cam.getView();

        glm::mat4 mvp = proj * view * model;

        pipeline.bind();

        pipeline.push(gfx::ShaderType::VERTEX, 0, sizeof(glm::mat4), &mvp);

        vkCmdDraw(ctx.getCommandBuffer(), 3, 1, 0, 0);

        ctx.endFrame();
    }

    pipeline.destroy();

    ctx.destroy();
    window.destroy();

    return 0;
}
