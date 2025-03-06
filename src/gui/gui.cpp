#include "gui.hpp"

namespace gui
{

void GUI::init(gfx::VulkanCtx &ctx)
{
    m_ctx = &ctx;

    m_pipeline = gfx::Pipeline::Builder(*m_ctx)
        .setShader(VK_SHADER_STAGE_VERTEX_BIT, "gui.vert.spv")
        .setShader(VK_SHADER_STAGE_FRAGMENT_BIT, "gui.frag.spv")
        .addDescriptorBinding({
            .binding = 0,
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .count = 1,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
        })
        .addDescriptorBinding({
            .binding = 1,
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .count = 1,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        })
        .addPushConstant(
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(PushConstant)
        )
        .setDepthTest(false)
        .setDepthWrite(false)
        .setBlending(true)
        .build();

    m_ubo.init(*m_ctx, sizeof(UniformBufferObject));
    m_texture.init(*m_ctx, "gui/icons.png");

    std::vector<gfx::DescriptorData> descriptorData = {
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .binding = 0,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .ubo = &m_ubo,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .binding = 1,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .texture = &m_texture,
        }
    };

    m_descriptorSet = m_pipeline.createDescriptorSet(descriptorData);

    m_text.init(*m_ctx);

    initGameElements();
}

void GUI::destroy()
{
    m_text.destroy();
    m_ubo.destroy();
    m_texture.destroy();
    m_pipeline.destroy();
}

void GUI::update()
{
    // Update elements
}

void GUI::render()
{
    VkExtent2D extent = m_ctx->getSwapChainExtent();
    
    glm::mat4 ortho = glm::ortho(
        -0.5f,
        static_cast<f32>(extent.width),
        -0.5f,
        static_cast<f32>(extent.height)
    );

    UniformBufferObject ubo{
        .ortho = ortho,
    };

    m_ubo.update(&ubo, sizeof(ubo));

    switch (m_gameStat.state)
    {

    case game::GameState::RUNNING:
        drawGameElements();
        break;

    case game::GameState::PAUSED:
        drawPauseElements();
        break;

    default:
        break;
    }
}

void GUI::draw(const Element &element)
{
    VkCommandBuffer cmd = m_ctx->getCommandBuffer();
    VkExtent2D extent = m_ctx->getSwapChainExtent();

    glm::vec2 pos = element.position;

    switch (element.anchor)
    {

    case Anchor::TOP_LEFT:
        break;

    case Anchor::TOP_RIGHT:
        pos.x = extent.width - pos.x - element.size.x;
        break;

    case Anchor::BOTTOM_LEFT:
        pos.y = extent.height - pos.y - element.size.y;
        break;

    case Anchor::BOTTOM_RIGHT:
        pos.x = extent.width - pos.x - element.size.x;
        pos.y = extent.height - pos.y - element.size.y;
        break;

    case Anchor::CENTER:
        pos.x = extent.width / 2.0f - element.size.x / 2.0f;
        pos.y = extent.height / 2.0f - element.size.y / 2.0f;
        break;
    }

    m_pipeline.bind();
    m_pipeline.bindDescriptorSet(m_descriptorSet);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(
        pos.x,
        pos.y,
        0.0f
    ));

    model = glm::scale(model, glm::vec3(
        element.size.x,
        element.size.y,
        1.0f
    ));

    PushConstant pc;
    pc.model = model;
    pc.uv = {
        element.uv.x / ATLAS_SIZE,
        element.uv.y / ATLAS_SIZE,
        element.uv.z / ATLAS_SIZE,
        element.uv.w / ATLAS_SIZE
    };

    m_pipeline.push(
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(pc),
        &pc
    );

    vkCmdDraw(cmd, 6, 1, 0, 0);
}

void GUI::initGameElements()
{
    Element crosshair = {
        .anchor = Anchor::CENTER,
        .position = {500.0f, 500.0f},
        .size = {64.0f, 64.0f},
        .uv = {0.0f, 0.0f, 16.0f, 16.0f},
    };

    m_elements["crosshair"] = crosshair;
}

void GUI::drawGameElements()
{
    std::string stat = "Minecraft Vulkan Clone ";
    stat += "(" + std::to_string(m_gameStat.fps) + " fps";
    stat += ", " + std::to_string(m_gameStat.updatedChunks) + " chunk updates)";

    m_text.draw(stat, {10.0f, 10.0f}, 24.0f);

    for (auto &[name, element] : m_elements) {
        draw(element);
    }
}

void GUI::drawPauseElements()
{
    m_text.draw("Game menu", {0.0f, -200.0f}, 24.0f, TextAlign::CENTER);

    for (auto &[name, element] : m_elements) {
        draw(element);
    }
}

} // namespace gui