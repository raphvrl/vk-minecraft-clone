#include "gui_manager.hpp"

namespace gui
{

void GUIManager::init(gfx::Device &device, gfx::TextureCache &textureCache)
{
    m_device = &device;
    m_textureCache = &textureCache;

    m_pipeline = gfx::Pipeline::Builder(device)
        .setShader("gui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT)
        .setShader("gui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
        .setPushConstant(sizeof(PushConstant))
        .setBlending(true)
        .build();

    loadConfig();
}

void GUIManager::destroy()
{
    m_pipeline.destroy();
    m_elements.clear();
}

void GUIManager::render(VkCommandBuffer cmd)
{
    m_pipeline.bind(cmd);

    for (const auto &[id, element] : m_elements) {
        if (element->visible) {
            renderElement(cmd, element);
        }
    }
}

void GUIManager::renderElement(
    VkCommandBuffer cmd,
    const std::shared_ptr<GUIElement> &element
)
{
    if (!element->visible) return;

    glm::vec2 pos = calculateAnchoredPosition(element);
    glm::vec2 size = element->size * m_guiScale;
    glm::vec4 uv = {
        element->uv.x / 256,
        element->uv.y / 256,
        element->uv.z / 256,
        element->uv.w / 256
    };

    PushConstant pc;
    pc.model = glm::mat4(1.0f);
    pc.model = glm::translate(pc.model, {pos.x, pos.y, 0.0f});
    pc.model = glm::scale(pc.model, {size.x, size.y, 1.0f});
    pc.uv = uv;
    pc.textureID = m_textureCache->getTextureID(element->texture);

    m_pipeline.bind(cmd);
    m_pipeline.push(cmd, pc);
    
    gfx::drawQuad(cmd);
}

void GUIManager::loadConfig()
{
    fs::path fullPath = CONFIG_PATH;

    if (!fs::exists(fullPath)) {
        throw std::runtime_error("Config file does not exist: " + fullPath.string());
    }

    std::ifstream file(fullPath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + fullPath.string());
    }

    json config = json::parse(file);

    clearAll();

    if (config.contains("elements") && config["elements"].is_array()) {
        for (const auto &elemJson : config["elements"]) {
            auto element = GUIElement::fromJson(elemJson);
            m_elements[element->id] = element;
        }
    }
}

void GUIManager::saveConfig()
{
    fs::path fullPath = CONFIG_PATH;

    json config;
    json elementsArray = json::array();

    for (const auto &[id, element] : m_elements) {
        elementsArray.push_back(element->toJson());
    }

    config["elements"] = elementsArray;

    fs::path dir = fs::path(CONFIG_PATH);
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
    }

    std::ofstream file(fullPath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file for writing: " + fullPath.string());
    }

    file << config.dump(4);
    file.close();
}

std::shared_ptr<GUIElement> GUIManager::getElement(const std::string &id) const
{
    auto it = m_elements.find(id);
    if (it != m_elements.end()) {
        return it->second;
    }
    return nullptr;
}

glm::vec2 GUIManager::calculateAnchoredPosition(
    const std::shared_ptr<GUIElement> &element
)
{
    VkExtent2D extent = m_device->getExtent();
    ElementAnchor anchor = element->anchor;
    glm::vec2 offset = element->position;
    glm::vec2 size = element->size * m_guiScale;
    glm::vec2 anchorPos;

    switch (anchor) {
        case ElementAnchor::TOP_LEFT:
            anchorPos = {0.0f, 0.0f};
            return anchorPos + offset;
            
        case ElementAnchor::TOP_CENTER:
            anchorPos = {extent.width / 2.0f - size.x / 2.0f, 0.0f};
            return glm::vec2(anchorPos.x + offset.x, anchorPos.y + offset.y);
            
        case ElementAnchor::TOP_RIGHT:
            anchorPos = {extent.width - size.x, 0.0f};
            return glm::vec2(anchorPos.x - offset.x, anchorPos.y + offset.y);
            
        case ElementAnchor::CENTER_LEFT:
            anchorPos = {0.0f, extent.height / 2.0f - size.y / 2.0f};
            return glm::vec2(anchorPos.x + offset.x, anchorPos.y + offset.y);
            
        case ElementAnchor::CENTER:
            anchorPos = {extent.width / 2.0f - size.x / 2.0f, extent.height / 2.0f - size.y / 2.0f};
            return anchorPos + offset;
            
        case ElementAnchor::CENTER_RIGHT:
            anchorPos = {extent.width - size.x, extent.height / 2.0f - size.y / 2.0f};
            return glm::vec2(anchorPos.x - offset.x, anchorPos.y + offset.y);
            
        case ElementAnchor::BOTTOM_LEFT:
            anchorPos = {0.0f, extent.height - size.y};
            return glm::vec2(anchorPos.x + offset.x, anchorPos.y - offset.y);
            
        case ElementAnchor::BOTTOM_CENTER:
            anchorPos = {extent.width / 2.0f - size.x / 2.0f, extent.height - size.y};
            return glm::vec2(anchorPos.x + offset.x, anchorPos.y - offset.y);
            
        case ElementAnchor::BOTTOM_RIGHT:
            anchorPos = {extent.width - size.x, extent.height - size.y};
            return glm::vec2(anchorPos.x - offset.x, anchorPos.y - offset.y);
    }

    return offset;
}

void GUIManager::addElement(const std::shared_ptr<GUIElement> &element)
{
    m_elements[element->id] = element;
}

void GUIManager::removeElement(const std::string &id)
{
    m_elements.erase(id);
}

void GUIManager::renameElement(const std::string &oldID, const std::string &newID)
{
    auto it = m_elements.find(oldID);
    if (it != m_elements.end()) {
        auto element = it->second;
        m_elements.erase(it);
        element->id = newID;
        m_elements[newID] = element;
    }
}

void GUIManager::clearAll()
{
    m_elements.clear();
}

void GUIManager::setElementVisible(const std::string &id, bool visible)
{
    auto element = getElement(id);
    if (element) {
        element->visible = visible;
    }
}

void GUIManager::setElementPosition(const std::string &id, const glm::vec2 &position)
{
    auto element = getElement(id);
    if (element) {
        element->position = position;
    }
}

void GUIManager::setElementSize(const std::string &id, const glm::vec2 &size)
{
    auto element = getElement(id);
    if (element) {
        element->size = size;
    }
}

} // namespace gui