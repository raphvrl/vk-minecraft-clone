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
    loadTexture("icons", "gui/icons.png");
    loadTexture("gui", "gui/gui.png");

    m_text.init(*m_ctx);
}

void GUI::destroy()
{
    m_text.destroy();
    m_ubo.destroy();

    for (auto &[_, texture] : m_textures) {
        texture.destroy();
    }

    m_pipeline.destroy();
}

void GUI::update(const glm::vec2 &point)
{
    for (auto &[_, button] : m_buttons) {
        button->update(point);
    }
}

void GUI::handleMouseClick()
{
    for (auto &[_, button] : m_buttons) {
        button->handleMouseClick();
    }
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

void GUI::initGameElements()
{
    Element crosshair = {
        .anchor = Anchor::CENTER,
        .pos = {500.0f, 500.0f},
        .size = {64.0f, 64.0f},
        .uv = {0.0f, 0.0f, 16.0f, 16.0f},
        .texture = "icons",
    };

    m_elements["crosshair"] = crosshair;
}

void GUI::initPauseElements()
{
    Element background = {
        .anchor = Anchor::CENTER,
        .pos = {0.0f, -100.0f},
        .size = {600.0f, 60.0f},
        .uv = {0.0f, 0.0f, 0.0f, 0.0f},
        .texture = "gui",
    };

    auto resumeButton = std::make_unique<Button>(
        this,
        "Back to game",
        background,
        m_resumeCallback
    );

    m_buttons["resume"] = std::move(resumeButton);

    background.pos.y += 100.0f;

    auto quitButton = std::make_unique<Button>(
        this,
        "Save and quit to title",
        background,
        m_quitCallback
    );

    m_buttons["quit"] = std::move(quitButton);
}

void GUI::draw(const Element &element)
{
    VkCommandBuffer cmd = m_ctx->getCommandBuffer();
    VkExtent2D extent = m_ctx->getSwapChainExtent();

    glm::vec2 pos = element.pos;

    switch (element.anchor) {
    case Anchor::TOP_LEFT:
        pos = element.pos;
        break;
        
    case Anchor::TOP_RIGHT:
        pos.x = extent.width - element.pos.x - element.size.x;
        pos.y = element.pos.y;
        break;
        
    case Anchor::BOTTOM_LEFT:
        pos.x = element.pos.x;
        pos.y = extent.height - element.pos.y - element.size.y;
        break;
        
    case Anchor::BOTTOM_RIGHT:
        pos.x = extent.width - element.pos.x - element.size.x;
        pos.y = extent.height - element.pos.y - element.size.y;
        break;
        
    case Anchor::CENTER:
        pos.x = (extent.width - element.size.x) / 2.0f + element.pos.x;
        pos.y = (extent.height - element.size.y) / 2.0f + element.pos.y;
        break;
        
    default:
        pos = element.pos;
        break;
    }

    m_pipeline.bind();
    m_pipeline.bindDescriptorSet(m_descriptorSets[element.texture]);

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
        element.uv.x,
        element.uv.y,
        element.uv.x + element.uv.z,
        element.uv.y + element.uv.w
    };

    pc.uv /= ATLAS_SIZE;

    m_pipeline.push(
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(pc),
        &pc
    );

    vkCmdDraw(cmd, 6, 1, 0, 0);
}

void GUI::loadTexture(const std::string &name, const std::string &path)
{
    m_textures[name].init(*m_ctx, path);

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
            .texture = &m_textures[name],
        }
    };

    m_descriptorSets[name] = m_pipeline.createDescriptorSet(descriptorData);
}

void GUI::drawGameElements()
{
    std::string stat = "Minecraft Vulkan Clone ";
    stat += "(" + std::to_string(m_gameStat.fps) + " fps";
    stat += ", " + std::to_string(m_gameStat.updatedChunks) + " chunk updates)";

    m_text.draw(stat, {10.0f, 10.0f}, 24.0f);

    for (auto &[_, element] : m_elements) {
        draw(element);
    }
}

void GUI::drawPauseElements()
{
    VkExtent2D extent = m_ctx->getSwapChainExtent();
    glm::vec2 textPos(
        extent.width / 2.0f,
        extent.height / 2.0f - 200.0f
    );

    m_text.draw("Game menu", textPos, 24.0f, TextAlign::CENTER);

    for (auto &[_, element] : m_elements) {
        draw(element);
    }

    for (auto &[_, button] : m_buttons) {
        button->render();
    }
}

} // namespace gui