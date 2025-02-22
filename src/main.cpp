#include "core/window/window.hpp"
#include "utils/drawer/box_drawer.hpp"
#include "utils/console/console.hpp"
#include "core/camera/camera.hpp"
#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/texture.hpp"
#include "world/chunk_mesh.hpp"
#include "world/block_registry.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <locale.h>
#endif

bool checkVulkanRuntime()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkInstance instance;
    VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);

    if (res != VK_SUCCESS) {
        const char* msg = 
            "Vulkan runtime not found!\n"
            "Please download and install from:\n"
            "https://vulkan.lunarg.com/sdk/home";
#ifdef _WIN32
        MessageBoxA(nullptr, msg, "Error", MB_ICONERROR | MB_OK);
#else
        printf("%s\n", msg);
#endif
        return false;
    }

    vkDestroyInstance(instance, nullptr);
    return true;
}

int main()
{
    #ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
    #else
        setlocale(LC_ALL, ".UTF-8");
    #endif

    if (!checkVulkanRuntime()) {
        return 1;
    }

    core::Window window;
    
    window.init(1280, 720, "Daskans ZIZI");
    window.setCursorVisible(false);

    gfx::VulkanCtx ctx;
    ctx.init(window);

    auto bindingDesc = wld::ChunkMesh::Vertex::getBindingDescription();
    auto attrDesc = wld::ChunkMesh::Vertex::getAttributeDescriptions();

    gfx::Pipeline pipeline = gfx::Pipeline::Builder(ctx)
        .setShader(gfx::ShaderType::VERTEX, "default.vert.spv")
        .setShader(gfx::ShaderType::FRAGMENT, "default.frag.spv")
        .addPushConstant(gfx::ShaderType::VERTEX, 0, sizeof(glm::mat4))
        .setVertexInput(
            &bindingDesc,
            attrDesc.data(),
            attrDesc.size()
        )
        .addDescriptorBinding({
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .count = 1,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT
        })
        .build();

    core::Camera cam;

    wld::Chunk chunk;
    for (u32 y = 0; y < wld::Chunk::CHUNK_HEIGHT; y++) {
        for (u32 z = 0; z < wld::Chunk::CHUNK_SIZE; z++) {
            for (u32 x = 0; x < wld::Chunk::CHUNK_SIZE; x++) {
                if (y < 4) {
                    chunk.setBlock(x, y, z, wld::BlockType::STONE);
                } else {
                    chunk.setBlock(x, y, z, wld::BlockType::AIR);
                }
            }
        }
    }
    
    wld::BlockRegistry blockRegistry;
    blockRegistry.load("assets/config/blocks.toml");

    wld::ChunkMesh chunkMesh;
    chunkMesh.init(ctx, blockRegistry);
    chunkMesh.generateMesh(chunk);

    glm::mat4 model(1.0f);

    gfx::Texture texture;
    texture.init(ctx, "blocks.png");

    VkDescriptorSet cobblestone = pipeline.createDescriptorSet(texture);

    while (window.isOpen()) {
        window.update();

        f32 dt = window.getDeltaTime();
        
        f32 speed = 10.0f * dt;

        if (window.isKeyPressed(core::Key::ESCAPE)) {
            break;
        }

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

        auto mouseRel = window.getMouseRel();
        cam.rotate(mouseRel.x, mouseRel.y);

        cam.update();

        if (!ctx.beginFrame()) {
            continue;
        }

        glm::mat4 proj = cam.getProj();
        glm::mat4 view = cam.getView();

        glm::mat4 mvp = proj * view * model;

        pipeline.bind();
        pipeline.bindDescriptorSet(cobblestone);

        pipeline.push(gfx::ShaderType::VERTEX, 0, sizeof(glm::mat4), &mvp);

        chunkMesh.draw();

        ctx.endFrame();
    }

    chunkMesh.destroy();
    
    texture.destroy();
    pipeline.destroy();

    ctx.destroy();
    window.destroy();

    return 0;
}
