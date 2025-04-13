#include "sky.hpp"

namespace wld
{

void Sky::init(gfx::Device &device)
{
    UNUSED(device);

    m_pipeline = gfx::Pipeline::Builder(device)
        .setShader("sky.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .setShader("sky.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .setDepthTest(false)
        .setDepthWrite(false)
        .setCullMode(VK_CULL_MODE_NONE)
        .build();
}

void Sky::destroy()
{
    m_pipeline.destroy();
}

void Sky::render(VkCommandBuffer cmd)
{
    m_pipeline.bind(cmd);
    vkCmdDraw(cmd, 36, 1, 0, 0);
}

} // namespace wld