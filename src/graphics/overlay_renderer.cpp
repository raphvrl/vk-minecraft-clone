#include "overlay_renderer.hpp"

namespace gfx
{

void OverlayRenderer::init(Device &device)
{
    m_device = &device;

    m_texture = m_device->loadImage(
        "assets/textures/water.png",
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
    );

    m_textureID = m_device->addTexture(m_texture);

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
    m_texture.destroy();
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