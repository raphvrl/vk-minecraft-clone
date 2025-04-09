#include "outline.hpp"

namespace wld
{

void Outline::init(gfx::Device &device, World &world)
{
    m_pipeline = gfx::Pipeline::Builder(device)
        .setShader("outline.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .setShader("outline.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .addPushConstantRange({
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(glm::mat4)
        })
        .setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST)
        .setCullMode(VK_CULL_MODE_NONE)
        .setBlending(true)
        .setDepthTest(true)
        .build();

    m_world = &world;
}

void Outline::destroy()
{
    m_pipeline.destroy();
}

void Outline::render(VkCommandBuffer cmd, const core::Camera &camera)
{
    Ray ray;
    ray.origin = camera.getPos();
    ray.direction = camera.getFront();

    RaycastResult result;

    if (m_world->raycast(ray, 4.0f, result)) {
        drawOutline(cmd, result.pos);
    }
}

void Outline::drawOutline(VkCommandBuffer cmd, const glm::ivec3 &pos)
{
    m_pipeline.bind(cmd);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), {pos.x, pos.y, pos.z});

    m_pipeline.push(
        cmd,
        VK_SHADER_STAGE_VERTEX_BIT,
        sizeof(model),
        &model
    );

    vkCmdDraw(cmd, 36, 1, 0, 0);
}

} // namespace wld