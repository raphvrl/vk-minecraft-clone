#include "sky.hpp"

namespace wld
{

void Sky::init(gfx::VulkanCtx &ctx)
{
    m_ctx = &ctx;

    m_pipeline = gfx::Pipeline::Builder(*m_ctx)
        .setShader(VK_SHADER_STAGE_VERTEX_BIT, "sky.vert.spv")
        .setShader(VK_SHADER_STAGE_FRAGMENT_BIT, "sky.frag.spv")
        .addPushConstant(
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(glm::mat4)
        )
        .setDepthTest(false)
        .setDepthWrite(false)
        .setCullMode(VK_CULL_MODE_NONE)
        .build();
}

void Sky::destroy()
{
    m_pipeline.destroy();
}

void Sky::render(const core::Camera &camera)
{
    VkCommandBuffer commandBuffer = m_ctx->getCommandBuffer();

    m_pipeline.bind();

    glm::mat4 view = glm::mat4(glm::mat3(camera.getView()));
    glm::mat4 viewProj = camera.getProj() * view;

    m_pipeline.push(
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(glm::mat4),
        &viewProj
    );

    vkCmdDraw(commandBuffer, 36, 1, 0, 0);
}

} // namespace wld