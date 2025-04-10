#include "gui.hpp"

namespace gui
{

void GUI::init(gfx::Device &device)
{
    m_device = &device;

    loadTexture("icons", "assets/textures/gui/icons.png");
    loadTexture("gui", "assets/textures/gui/gui.png");

    initGameElements();
    initPauseElements();

    m_pipeline = gfx::Pipeline::Builder(device)
        .setShader("gui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .setShader("gui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .setPushConstant(sizeof(PushConstant))
        .setDepthTest(false)
        .setDepthWrite(false)
        .setBlending(true)
        .build();

    m_text.init(device);
}

void GUI::destroy()
{
    for (auto &[_, texture] : m_textures) {
        texture.first.destroy();
    }

    m_text.destroy();

    m_pipeline.destroy();
}

void GUI::update(const glm::vec2 &point)
{
    auto state = m_gameStat.state;

    if (state == game::GameState::PAUSED) {
        for (auto &[_, button] : m_buttons) {
            button->update(point);
        }
    }
}

void GUI::handleMouseClick()
{
    for (auto &[_, button] : m_buttons) {
        button->handleMouseClick();
    }
}

void GUI::render(VkCommandBuffer cmd)
{
    switch (m_gameStat.state)
    {

    case game::GameState::RUNNING:
        drawGameElements(cmd);
        break;

    case game::GameState::PAUSED:
        drawPauseElements(cmd);
        break;

    default:
        break;
    }
}

void GUI::initGameElements()
{
    Element crosshair = {
        .anchor = Anchor::CENTER,
        .pos = {0.0f, 0.0f},
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
        .size = {800.0f, 80.0f},
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

void GUI::draw(VkCommandBuffer cmd, const Element &element)
{
    VkExtent2D extent = m_device->getExtent();

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

    m_pipeline.bind(cmd);

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
    pc.textureID = m_textures[element.texture].second;

    pc.uv /= ATLAS_SIZE;

    m_pipeline.push(cmd, pc);

    vkCmdDraw(cmd, 6, 1, 0, 0);
}

void GUI::loadTexture(const std::string &name, const std::string &path)
{
    m_textures[name].first = m_device->loadImage(
        path,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT
    );

    m_textures[name].second = m_device->addTexture(m_textures[name].first);

    m_device->update();
}

void GUI::drawGameElements(VkCommandBuffer cmd)
{
    std::string stat = "Minecraft Vulkan Clone ";
    stat += "(" + std::to_string(m_gameStat.fps) + " fps";
    stat += ", " + std::to_string(m_gameStat.updatedChunks) + " chunk updates)";

    m_text.draw(cmd, stat, {10.0f, 10.0f}, 32.0f);

    for (auto &[_, element] : m_elements) {
        draw(cmd, element);
    }
}

void GUI::drawPauseElements(VkCommandBuffer cmd)
{
    VkExtent2D extent = m_device->getExtent();
    glm::vec2 textPos(
        extent.width / 2.0f,
        extent.height / 2.0f - 350.0f
    );

    m_text.draw(cmd, "Game menu", textPos, 32.0f, TextAlign::CENTER);

    for (auto &[_, element] : m_elements) {
        draw(cmd, element);
    }

    for (auto &[_, button] : m_buttons) {
        button->draw(cmd);
    }
}

} // namespace gui