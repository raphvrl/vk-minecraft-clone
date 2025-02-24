#include "sky.hpp"

namespace wld
{

void Sky::init(gfx::VulkanCtx &ctx)
{
    m_ctx = &ctx;

    m_pipeline = gfx::Pipeline::Builder(*m_ctx)
        .setShader(gfx::ShaderType::VERTEX, "sky.vert.spv")
        .setShader(gfx::ShaderType::FRAGMENT, "sky.frag.spv")
        .setDepthTest(false)
        .setDepthWrite(false)
        .build();
}

void Sky::destroy()
{
    m_pipeline.destroy();
}

void Sky::render()
{
    VkCommandBuffer VkCommandBuffer = m_ctx->getCommandBuffer();

    m_pipeline.bind();

    vkCmdDraw(VkCommandBuffer, 3, 1, 0, 0);
}

} // namespace wld