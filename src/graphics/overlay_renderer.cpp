#include "overlay_renderer.hpp"

namespace gfx
{

void OverlayRenderer::init(VulkanCtx &ctx)
{
    m_ctx = &ctx;

    m_pipeline = Pipeline::Builder(*m_ctx)
        .setShader(VK_SHADER_STAGE_VERTEX_BIT, "overlay.vert.spv")
        .setShader(VK_SHADER_STAGE_FRAGMENT_BIT, "overlay.frag.spv")
        .addDescriptorBinding({
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .count = 1,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT
        })
        .setDepthTest(false)
        .setDepthWrite(false)
        .setBlending(true)
        .build();


    createWater();
}

void OverlayRenderer::destroy()
{
    m_pipeline.destroy();
    m_waterTexture.destroy();
}

void OverlayRenderer::render()
{
    VkCommandBuffer cmd = m_ctx->getCommandBuffer();

    if (m_water) {
        m_pipeline.bind();
        m_pipeline.bindDescriptorSet(m_waterSet);

        vkCmdDraw(cmd, 6, 1, 0, 0);
    }
}

void OverlayRenderer::createWater()
{
    m_waterTexture.init(*m_ctx, "water.png");

    std::vector<DescriptorData> data = {
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .binding = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .texture = &m_waterTexture
        }
    };

    m_waterSet = m_pipeline.createDescriptorSet(data);
}

} // namespace gfx