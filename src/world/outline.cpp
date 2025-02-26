#include "outline.hpp"

namespace wld
{

void Outline::init(gfx::VulkanCtx &ctx, World &world)
{
    m_ctx = &ctx;

    m_pipeline = gfx::Pipeline::Builder(*m_ctx)
        .setShader(VK_SHADER_STAGE_VERTEX_BIT, "outline.vert.spv")
        .setShader(VK_SHADER_STAGE_FRAGMENT_BIT, "outline.frag.spv")
        .addDescriptorBinding({
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .count = 1,
            .stage = VK_SHADER_STAGE_VERTEX_BIT
        })
        .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
        .setCullMode(VK_CULL_MODE_NONE)
        .setLineWidth(2.0f)
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

    m_world = &world;
}

void Outline::destroy()
{
    m_pipeline.destroy();
    m_ubo.destroy();
}

void Outline::render(const core::Camera &camera)
{
    Ray ray;
    ray.origin = camera.getPos();
    ray.direction = camera.getFront();

    RaycastResult result;

    if (m_world->raycast(ray, 5.0f, result)) {
        drawOutline(camera, result.pos);
    }
}

void Outline::drawOutline(const core::Camera &camera, const glm::ivec3 &pos)
{
    m_pipeline.bind();

    m_pipeline.bindDescriptorSet(m_descriptorSet);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(pos));

    UniformBufferObject uniformBuffer = {
        .model = model,
        .view = camera.getView(),
        .proj = camera.getProj(),
    };

    m_ubo.update(&uniformBuffer, sizeof(UniformBufferObject));

    vkCmdDraw(m_ctx->getCommandBuffer(), 24, 1, 0, 0);
}

} // namespace wld