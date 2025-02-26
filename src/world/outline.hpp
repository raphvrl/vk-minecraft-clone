#pragma once

#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/uniform_buffer.hpp"
#include "core/camera/camera.hpp"
#include "world/world.hpp"


namespace wld
{

class Outline
{

public:
    void init(gfx::VulkanCtx &ctx, World &world);
    void destroy();

    void render(const core::Camera &camera);

private:
    gfx::VulkanCtx *m_ctx;
    gfx::Pipeline m_pipeline;

    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };

    gfx::UniformBuffer m_ubo;
    VkDescriptorSet m_descriptorSet;

    World *m_world;

    void drawOutline(
        const core::Camera &camera,
        const glm::ivec3 &pos
    );
};

} // namespace wld