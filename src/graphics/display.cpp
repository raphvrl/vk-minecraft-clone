#include "display.hpp"

namespace gfx
{

void Display::init(Device &device)
{
    m_device = &device;

    VkExtent2D extent = device.getExtent();
    VkFormat format = device.getSwapchain().getFormat();

    m_framebuffer.init(
        device,
        extent.width,
        extent.height,
        format,
        device.getDepthFormat()
    );

    m_pipeline = Pipeline::Builder(device)
        .setShader("display.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .setShader("display.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .setPushConstant(sizeof(PushConstant))
        .build();

    m_pc.textureID = 0;
    m_pc.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
}

void Display::destroy()
{
    m_framebuffer.destroy();
    m_pipeline.destroy();
}

void Display::resize(u32 width, u32 height)
{
    m_framebuffer.resize(width, height);
}

void Display::begin(VkCommandBuffer cmd)
{
    VkExtent2D extent = m_device->getExtent();
    resize(extent.width, extent.height);

    m_framebuffer.begin(cmd);
}

void Display::end(VkCommandBuffer cmd)
{
    m_framebuffer.end(cmd);
}

void Display::draw(VkCommandBuffer cmd)
{
    m_pipeline.bind(cmd);

    m_pc.textureID = m_framebuffer.getTextureID();
    m_pipeline.push(cmd, m_pc);

    vkCmdDraw(cmd, 6, 1, 0, 0);
}

} // namespace gfx