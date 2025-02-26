#pragma once

#include <glm/ext.hpp>

#include <array>

#include "graphics/vulkan_ctx.hpp"
#include "graphics/pipeline.hpp"
#include "graphics/uniform_buffer.hpp"
#include "core/camera/camera.hpp"

namespace wld
{

class Sky
{

public:
    void init(gfx::VulkanCtx &ctx);
    void destroy();

    void render(const core::Camera &camera);
private:
    gfx::VulkanCtx *m_ctx;
    gfx::Pipeline m_pipeline;

    struct UniformBufferObject
    {
        glm::mat4 view;
        glm::mat4 proj;
    };

    gfx::UniformBuffer m_ubo;
    VkDescriptorSet m_descriptorSet;
};

} // namespace wld