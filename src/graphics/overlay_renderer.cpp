#include "overlay_renderer.hpp"

namespace gfx
{

void OverlayRenderer::init(Device &device, TextureCache &TextureCache)
{
    m_device = &device;

    m_textureID = TextureCache.getTextureID("water");

    m_pipeline = Pipeline::Builder(*m_device)
        .setShader("overlay.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .setShader("overlay.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .setPushConstant(sizeof(u32))
        .setDepthTest(false)
        .setDepthWrite(false)
        .setBlending(true)
        .build();
}

void OverlayRenderer::destroy()
{
    m_pipeline.destroy();
}

void OverlayRenderer::render(VkCommandBuffer cmd)
{
    if (m_water) {
        m_pipeline.bind(cmd);

        m_pipeline.push(cmd, m_textureID);

        vkCmdDraw(cmd, 6, 1, 0, 0);
    }
}

} // namespace gfx