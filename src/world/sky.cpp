#include "sky.hpp"

namespace wld
{

void Sky::init(gfx::VulkanCtx &ctx)
{
    m_ctx = &ctx;

    m_pipeline = gfx::Pipeline::Builder(*m_ctx)
        .setShader(VK_SHADER_STAGE_VERTEX_BIT, "sky.vert.spv")
        .setShader(VK_SHADER_STAGE_FRAGMENT_BIT, "sky.frag.spv")
        .addDescriptorBinding({
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .count = 1,
            .stage = VK_SHADER_STAGE_VERTEX_BIT
        })
        .setDepthTest(false)
        .setDepthWrite(false)
        .setCullMode(VK_CULL_MODE_NONE)
        .build();

    m_ubo.init(*m_ctx, sizeof(UniformBufferObject));

    std::vector<gfx::DescriptorData> descriptors = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .binding = 0,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .ubo = &m_ubo
        }
    };

    m_descriptorSet = m_pipeline.createDescriptorSet(descriptors);
}

void Sky::destroy()
{
    m_ubo.destroy();
    m_pipeline.destroy();
}

void Sky::render(const core::Camera &camera)
{
    VkCommandBuffer commandBuffer = m_ctx->getCommandBuffer();

    m_pipeline.bind();
    m_pipeline.bindDescriptorSet(m_descriptorSet);

    UniformBufferObject ubo {
        .view = camera.getView(),
        .proj = camera.getProj()
    };

    m_ubo.update(&ubo, sizeof(ubo));

    vkCmdDraw(commandBuffer, 36, 1, 0, 0);
}

} // namespace wld