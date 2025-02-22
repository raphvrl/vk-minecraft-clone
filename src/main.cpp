#include "core/window/window.hpp"
#include "utils/drawer/box_drawer.hpp"
#include "utils/console/console.hpp"
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
        .build();

    while (window.isOpen()) {
        window.update();

        if (!ctx.beginFrame()) {
            continue;
        }

        pipeline.bind();

        vkCmdDraw(ctx.getCommandBuffer(), 3, 1, 0, 0);

        ctx.endFrame();
    }

    pipeline.destroy();

    ctx.destroy();
    window.destroy();

    return 0;
}
